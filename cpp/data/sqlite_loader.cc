#include "data/sqlite_loader.h"

#include <sqlite3.h>
#include <codecvt>
#include <locale>
#include <string>
#include <unordered_map>
#include "base/error_code.h"
#include "data/types/properties.h"
#include "glog/logging.h"

namespace monster_avengers {

namespace {
typedef int (*Callback)(void *, int, char **, char **);
// using Callback = std::function<int(void *, int, char **, char **)>;

void RunSQLiteQuery(sqlite3 *database, Data *data, const char *query,
                    Callback callback) {
  char *error = nullptr;

  int status = sqlite3_exec(database, query, callback,
                            static_cast<void *>(data), &error);

  if (SQLITE_OK != status) {
    LOG(ERROR) << "Sqlite Query Error" << error;
    sqlite3_free(error);
    LOG(FATAL) << "Terminate because the previous error.";
  }
}

template <typename ValueType>
ValueType ConvertTo(const std::string &original) {
  static_assert("Not implemented.");
}

template <>
int ConvertTo<int>(const std::string &original) {
  return std::stoi(original);
}

template <>
std::wstring ConvertTo<std::wstring>(const std::string &original) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf16conv;
  std::wstring result = utf16conv.from_bytes(original);
  return result;
}

template <>
Range ConvertTo<Range>(const std::string &original) {
  switch (std::stoi(original)) {
    case 1:
      return BLADEMASTER;
    case 2:
      return GUNNER;
    default:
      return RANGE_BOTH;
  }
}

template <>
Part ConvertTo<Part>(const std::string &original) {
  switch (std::stoi(original)) {
    case 1:
      return HEAD;
    case 2:
      return BODY;
    case 3:
      return HANDS;
    case 4:
      return WAIST;
    case 5:
      return FEET;
    default:
      return PART_UNKNOWN;
  }
}

template <>
Gender ConvertTo<Gender>(const std::string &original) {
  switch (std::stoi(original)) {
    case 1:
      return MALE;
    case 2:
      return FEMALE;
    default:
      return GENDER_BOTH;
  }
}

class Row {
 public:
  Row(int argc, char **argv, char **caption) : result_() {
    for (int i = 0; i < argc; ++i) {
      if (nullptr != argv[i]) {
        result_[caption[i]] = argv[i];
      } else {
        result_[caption[i]] = "";
      }
    }
  }

  template <typename ValueType>
  ValueType Get(const std::string &key) {
    return ConvertTo<ValueType>(result_[key]);
  }

 private:
  std::unordered_map<std::string, std::string> result_;
};
}  // namespace

::base::Status LoadFromSqlite(const std::string &path, Data *data) {
  sqlite3 *database;
  if (SQLITE_OK != sqlite3_open(path.c_str(), &database)) {
    return ::base::Status(::base::error::READ_ERROR,
                          "Failed to open the specified sqlite file at ", path,
                          sqlite3_errmsg(database));
  }

  // Fetch all the skill trees.
  RunSQLiteQuery(
      database, data,
      "SELECT SklTree_ID AS dex_id, "
      "       SklTree_Name_0 AS eng, "
      "       SklTree_Name_1 AS chs, "
      "       SklTree_Name_3 AS jpn "
      "FROM ID_SklTree_Name;",
      [](void *data, int argc, char **argv, char **caption) {
        Data &result = *static_cast<Data *>(data);

        Row row(argc, argv, caption);

        // Insert to skill_trees.
        result.skill_trees.emplace_back();
        SkillTree &skill_tree = result.skill_trees.back();
        skill_tree.id = static_cast<int>(result.skill_trees.size() - 1);
        skill_tree.dex_id = row.Get<int>("dex_id");

        skill_tree.name.eng = row.Get<std::wstring>("eng");
        skill_tree.name.chs = row.Get<std::wstring>("chs");
        skill_tree.name.jpn = row.Get<std::wstring>("jpn");

        // Update the translator.
        result.skill_tree_dex.Update(skill_tree.id, skill_tree.dex_id);

        return 0;
      });

  // Fill skill trees with (skill, points) pairs.
  RunSQLiteQuery(database, data,
                 "SELECT DB_Skl.Skl_ID AS skill_id, "
                 "       SklTree_ID AS dex_id, "
                 "       Pt AS points, "
                 "       Skl_Name_0 AS eng, "
                 "       Skl_Name_1 AS chs, "
                 "       Skl_Name_3 AS jpn "
                 "FROM DB_Skl "
                 "INNER JOIN ID_Skl_Name ON "
                 "    DB_Skl.Skl_ID = ID_Skl_Name.Skl_ID ",
                 [](void *data, int argc, char **argv, char **caption) {
                   Data &result = *static_cast<Data *>(data);

                   Row row(argc, argv, caption);

                   int id =
                       result.skill_tree_dex.FromDex(row.Get<int>("dex_id"));
                   SkillTree &skill_tree = result.skill_trees[id];
                   int points = row.Get<int>("points");

                   LangText name;
                   name.eng = row.Get<std::wstring>("eng");
                   name.chs = row.Get<std::wstring>("chs");
                   name.jpn = row.Get<std::wstring>("jpn");

                   if (points > 0) {
                     skill_tree.positives.emplace_back(std::move(name), points);
                   } else {
                     skill_tree.negatives.emplace_back(std::move(name), points);
                   }

                   return 0;
                 });

  // Fill Jewel list.
  RunSQLiteQuery(
      database, data,
      "SELECT Jew_ID AS dex_id, "
      "       Itm_Name_0 AS eng, "
      "       Itm_Name_1 AS chs, "
      "       Itm_Name_3 AS jpn, "
      "       Slot AS slots, "
      "       SklTree1_ID AS skill_id1, "
      "       SklTree1_Pt AS points1, "
      "       SklTree2_ID AS skill_id2, "
      "       SklTree2_Pt AS points2 "
      "FROM DB_JEW "
      "INNER JOIN ID_Itm_Name "
      "ON ID_Itm_Name.Itm_ID = DB_Jew.Itm_ID "
      "GROUP BY DB_Jew.Itm_ID "
      "ORDER BY DB_Jew.Itm_ID",
      [](void *data, int argc, char **argv, char **caption) {
        Data &result = *static_cast<Data *>(data);

        Row row(argc, argv, caption);

        int id = static_cast<int>(result.jewels.size());
        result.jewels.emplace_back();
        Jewel &jewel = result.jewels.back();

        jewel.id = id;
        jewel.dex_id = row.Get<int>("dex_id");
        jewel.name.eng = row.Get<std::wstring>("eng");
        jewel.name.chs = row.Get<std::wstring>("chs");
        jewel.name.jpn = row.Get<std::wstring>("jpn");
        jewel.slots = row.Get<int>("slots");
        int skill_id = result.skill_tree_dex.FromDex(row.Get<int>("skill_id1"));
        jewel.effect_list.emplace_back(skill_id, row.Get<int>("points1"));
        skill_id = result.skill_tree_dex.FromDex(row.Get<int>("skill_id2"));
        jewel.effect_list.emplace_back(skill_id, row.Get<int>("points2"));

        result.jewel_dex.Update(jewel.id, jewel.dex_id);

        return 0;
      });

  RunSQLiteQuery(database, data,
                 "SELECT DB_Amr.Amr_ID AS dex_id, "
                 "       ID_Amr_Name.Amr_Name_0 AS eng, "
                 "       ID_Amr_Name.Amr_Name_1 AS chs, "
                 "       ID_Amr_Name.Amr_Name_3 AS jpn, "
                 "       Part as part, "
                 "       Slot AS slots, "
                 "       BorG AS range, "
                 "       MorF AS gender, "
                 "       Rare AS rare, "
                 "       Def AS min_def, "
                 "       MaxDef AS max_def, "
                 "       Res_Fire AS fire, "
                 "       Res_Thunder AS thunder, "
                 "       Res_Dragon AS dragon, "
                 "       Res_Water AS water, "
                 "       Res_Ice AS ice "
                 "FROM DB_Amr "
                 "INNER JOIN ID_Amr_Name ON "
                 "ID_Amr_Name.Amr_ID = DB_Amr.Amr_ID "
                 "ORDER BY DB_Amr.Amr_ID",
                 [](void *data, int argc, char **argv, char **caption) {
                   Data &result = *static_cast<Data *>(data);

                   Row row(argc, argv, caption);

                   int id = static_cast<int>(result.armors.size());
                   result.armors.emplace_back();
                   Armor &armor = result.armors.back();

                   armor.id = id;
                   armor.dex_id = row.Get<int>("dex_id");
                   armor.part = row.Get<Part>("part");
                   armor.name.eng = row.Get<std::wstring>("eng");
                   armor.name.chs = row.Get<std::wstring>("chs");
                   armor.name.jpn = row.Get<std::wstring>("jpn");
                   armor.range = row.Get<Range>("range");
                   armor.gender = row.Get<Gender>("gender");
                   armor.slots = row.Get<int>("slots");
                   armor.rare = row.Get<int>("rare");
                   armor.min_defense = row.Get<int>("min_def");
                   armor.max_defense = row.Get<int>("max_def");

                   armor.resist.fire = row.Get<int>("fire");
                   armor.resist.thunder = row.Get<int>("thunder");
                   armor.resist.dragon = row.Get<int>("dragon");
                   armor.resist.water = row.Get<int>("water");
                   armor.resist.ice = row.Get<int>("ice");

                   result.armor_dex.Update(armor.id, armor.dex_id);

                   return 0;
                 });

  RunSQLiteQuery(
      database, data,
      "SELECT Amr_ID AS dex_id, "
      "       SklTree_ID AS skill_tree_id, "
      "       Pt AS points "
      "FROM DB_SklTreetoAmr ",
      [](void *data, int argc, char **argv, char **caption) {
        Row row(argc, argv, caption);

        Data &result = *static_cast<Data *>(data);
        int id = result.armor_dex.FromDex(row.Get<int>("dex_id"));
        int skill_tree_id =
            result.skill_tree_dex.FromDex(row.Get<int>("skill_tree_id"));

        Armor &armor = result.armors[id];

        armor.effect_list.emplace_back(skill_tree_id, row.Get<int>("points"));

        return 0;
      });

  sqlite3_close(database);
  return ::base::Status::OK();
}

}  // namespace monster_avengers
