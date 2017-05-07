#include "data/sqlite_loader.h"

#include <sqlite3.h>
#include <codecvt>
#include <locale>
#include <string>
#include <unordered_map>
#include "base/error_code.h"
#include "glog/logging.h"

namespace monster_avengers {
namespace data {

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

  sqlite3_close(database);
  return ::base::Status::OK();
}

}  // namespace data
}  // namespace monster_avengers
