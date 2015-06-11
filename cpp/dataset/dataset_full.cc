#include <string>
#include <sqlite3.h>
#include <unordered_map>
#include <vector>

#include "dataset.h"
#include "supp/helpers.h"

namespace monster_avengers {

namespace dataset {

namespace {

typedef int (CallbackPtr)(void*, int, char**, char**);

void RunSQLiteQuery(sqlite3 *database, const char *query,
                    CallbackPtr *callback) {
  char *error = nullptr;
  
  int status = sqlite3_exec(database, query,
                            callback,
                            nullptr, &error);

  if (SQLITE_OK != status) {
    Log(FATAL, L"SQLite Query Error: %s\n", error);
    sqlite3_free(error);
    CHECK(false);
  }
}

void GetRow(int argc, char **argv, char **caption,
            std::unordered_map<std::string, std::string> *row) {
  for (int i = 0; i < argc; ++i) {
    (*row)[caption[i]] = argv[i];
  }
}

}  // namespace

ReindexedTable<Armor> Data::armors_;
ReindexedTable<Jewel> Data::jewels_;
ReindexedTable<SkillSystem> Data::skills_;
ReindexedTable<Item> Data::items_;
ReindexedTable<ArmorAddon> Data::armor_addons_;
ReindexedTable<JewelAddon> Data::jewel_addons_;
ReindexedTable<SkillSystemAddon> Data::skill_addons_;

void Data::Initialize(const std::string &spec) {
  armors_.ResetIndexMap();
  jewels_.ResetIndexMap();
  skills_.ResetIndexMap();
  
  items_.ResetIndexMap();
  armor_addons_.ResetIndexMap(&armors_);
  jewel_addons_.ResetIndexMap(&jewels_);
  skill_addons_.ResetIndexMap(&skills_);

  sqlite3 *database;
  CHECK(!sqlite3_open(spec.c_str(), &database));
  RunSQLiteQuery(database,
                 "SELECT ID_SklTree_Name.SklTree_ID AS external_id, "
                 "SklTree_Name_0 AS en, "
                 "SklTree_Name_1 AS zh, "
                 "SklTree_Name_3 AS jp, "
                 "Skl_Name_0 AS sub_en, "
                 "Skl_Name_1 AS sub_zh, "
                 "Skl_Name_3 AS sub_jp, "
                 "Pt AS points "
                 "FROM ID_SklTree_Name "
                 "INNER JOIN DB_Skl ON "
                 "ID_SklTree_Name.SklTree_ID = DB_Skl.SklTree_ID "
                 "INNER JOIN ID_Skl_Name ON "
                 "ID_Skl_Name.Skl_ID = DB_Skl.Skl_ID "
                 "ORDER BY ID_SklTree_Name.SklTree_ID;",
                 [](void *data, int argc, char **argv, char **caption) {
                   std::unordered_map<std::string, std::string> row;
                   GetRow(argc, argv, caption, &row);
                   int external_id = std::stoi(row["external_id"]);
                   if (skills_.HasExternalId(external_id)) {
                     SkillSystem &skill_system = skills_.Select(external_id);
                     SkillSystemAddon &addon = skill_addons_.Select(external_id);
                     skill_system.skills.emplace_back();
                     skill_system.skills.back().points = std::stoi(row["points"]);
                     addon.skill_names.emplace_back();
                     addon.skill_names.back()[ENGLISH] = row["sub_en"];
                     addon.skill_names.back()[ENGLISH] = row["sub_zh"];
                     addon.skill_names.back()[ENGLISH] = row["sub_jp"];
                   } else {
                     SkillSystem skill_system;
                     skill_system.skills.emplace_back();
                     skill_system.skills[0].points = std::stoi(row["points"]);
                     skills_.Add(skill_system, external_id);
                     SkillSystemAddon addon;
                     addon.name[ENGLISH] = row["en"];
                     addon.name[CHINESE] = row["zh"];
                     addon.name[JAPANESE] = row["jp"];
                     addon.skill_names.emplace_back();
                     addon.skill_names[0][ENGLISH] = row["sub_en"];
                     addon.skill_names[0][CHINESE] = row["sub_zh"];
                     addon.skill_names[0][JAPANESE] = row["sub_jp"];
                     skill_addons_.Update(addon, external_id);
                   }
                   return 0;
                 });
}

}  // namespace dataset

}  // namespace monster_avengers
