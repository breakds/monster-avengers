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

ArmorPart ParsePart(const std::string &input) {
  switch (std::stoi(input)) {
    case 1: return HEAD;
    case 2: return BODY;
    case 3: return HANDS;
    case 4: return WAIST;
    case 5: return FEET;
    default: CHECK(false); return HEAD;
  }
}

WeaponType ParseWeaponType(const std::string &input) {
  switch (std::stoi(input)) {
    case 1: return WEAPON_TYPE_MELEE;
    case 2: return WEAPON_TYPE_RANGE;
    default: return WEAPON_TYPE_BOTH;
  }
}

Gender ParseGender(const std::string &input) {
  switch (std::stoi(input)) {
    case 1: return GENDER_MALE;
    case 2: return GENDER_FEMALE;
    default: return GENDER_BOTH;
  }
}

}  // namespace

void Data::LoadSQLite(const std::string &spec) {
  armors_.ResetIndexMap();
  jewels_.ResetIndexMap();
  skills_.ResetIndexMap();
  
  items_.ResetIndexMap();
  armor_addons_.ResetIndexMap(&armors_);
  jewel_addons_.ResetIndexMap(&jewels_);
  skill_addons_.ResetIndexMap(&skills_);

  sqlite3 *database;
  CHECK(!sqlite3_open(spec.c_str(), &database));

  // Skills
  RunSQLiteQuery(database,
                 "SELECT ID_SklTree_Name.SklTree_ID AS external_id, "
                 "       SklTree_Name_0 AS en, "
                 "       SklTree_Name_1 AS zh, "
                 "       SklTree_Name_3 AS jp, "
                 "       Skl_Name_0 AS sub_en, "
                 "       Skl_Name_1 AS sub_zh, "
                 "       Skl_Name_3 AS sub_jp, "
                 "       Pt AS points "
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

  // Jewels
  RunSQLiteQuery(database,
                 "SELECT Jew_ID AS external_id, "
                 "       DB_Jew.Itm_ID AS item_id, "
                 "       Itm_Name_0 AS en, "
                 "       Itm_Name_1 AS zh, "
                 "       Itm_Name_3 AS jp, "
                 "       Slot AS slots, "
                 "       SklTree1_ID AS skill_id_1, "
                 "       SklTree1_Pt AS points_1, "
                 "       SklTree2_ID AS skill_id_2, "
                 "       SklTree2_Pt AS points_2 "
                 "FROM DB_Jew "
                 "INNER JOIN ID_Itm_Name ON "
                 "ID_Itm_Name.Itm_ID = DB_Jew.Itm_ID "
                 "WHERE DB_Jew.Itm_ID < 1774 "
                 "GROUP BY DB_Jew.Itm_ID "
                 "ORDER BY Itm_Name_0",
                 [](void *data, int argc, char **argv, char **caption) {
                   std::unordered_map<std::string, std::string> row;
                   GetRow(argc, argv, caption, &row);
                   int external_id = std::stoi(row["external_id"]);
                   Jewel jewel;
                   jewel.slots = std::stoi(row["slots"]);
                   if ("-1" != row["skill_id_1"]) {
                     int skill_id = std::stoi(row["skill_id_1"]);
                     CHECK(skills_.HasExternalId(skill_id));
                     jewel.effects.emplace_back();
                     jewel.effects.back().id = skills_.Internalize(skill_id);
                     jewel.effects.back().points = std::stoi(row["points_1"]);
                   }
                   if ("-1" != row["skill_id_2"]) {
                     int skill_id = std::stoi(row["skill_id_2"]);
                     CHECK(skills_.HasExternalId(skill_id));
                     jewel.effects.emplace_back();
                     jewel.effects.back().id = skills_.Internalize(skill_id);
                     jewel.effects.back().points = std::stoi(row["points_2"]);
                   }
                   jewels_.Add(jewel, external_id);
                   JewelAddon addon;
                   addon.name[ENGLISH] = row["en"];
                   addon.name[CHINESE] = row["zh"];
                   addon.name[JAPANESE] = row["jp"];
                   jewel_addons_.Update(addon, external_id);
                   return 0;
                 });

  // Items
  RunSQLiteQuery(database,
                 "SELECT Itm_ID AS external_id, "
                 "       Itm_Name_0 AS en, Itm_Name_1 AS zh, "
                 "       Itm_Name_3 AS jp "
                 "FROM ID_Itm_Name "
                 "WHERE Itm_ID > 0 "
                 "ORDER BY Itm_ID;",
                 [](void *data, int argc, char **argv, char **caption) {
                   std::unordered_map<std::string, std::string> row;
                   GetRow(argc, argv, caption, &row);

                   int external_id = std::stoi(row["external_id"]);
                   Item item;
                   item.name[ENGLISH] = row["en"];
                   item.name[CHINESE] = row["zh"];
                   item.name[JAPANESE] = row["jp"];
                   items_.Add(item, external_id);
                   return 0;
                 });

  // Armor Properties
  RunSQLiteQuery(database,
                 "SELECT DB_Amr.Amr_ID AS external_id, "
                 "       ID_Amr_Name.Amr_Name_0 AS en, "
                 "       ID_Amr_Name.Amr_Name_1 AS zh, "
                 "       ID_Amr_Name.Amr_Name_3 AS jp, "
                 "       Part AS part, Slot AS slots, Rare AS rare, "
                 "       BorG AS weapon_type, MorF AS gender, "
                 "       Def AS min_def, MaxDef AS max_def, Res_Fire AS fire, "
                 "       Res_Thunder AS thunder, Res_Dragon AS dragon, "
                 "       Res_Water AS water, Res_Ice AS ice "
                 "FROM DB_Amr "
                 "INNER JOIN ID_Amr_Name ON "
                 "ID_Amr_Name.Amr_ID = DB_Amr.Amr_ID "
                 "ORDER BY DB_Amr.Amr_ID; ",
                 [](void *data, int argc, char **argv, char **caption) {
                   std::unordered_map<std::string, std::string> row;
                   GetRow(argc, argv, caption, &row);

                   int external_id = std::stoi(row["external_id"]);
                   Armor armor;
                   armor.part = ParsePart(row["part"]);
                   armor.weapon_type = ParseWeaponType(row["weapon_type"]);
                   armor.gender = ParseGender(row["gender"]);
                   armor.rare = std::stoi(row["rare"]);
                   armor.min_defense = std::stoi(row["min_def"]);
                   armor.max_defense = std::stoi(row["max_def"]);
                   armor.resistence.fire = std::stoi(row["fire"]);
                   armor.resistence.thunder = std::stoi(row["thunder"]);
                   armor.resistence.dragon = std::stoi(row["dragon"]);
                   armor.resistence.water = std::stoi(row["water"]);
                   armor.resistence.ice = std::stoi(row["ice"]);
                   armor.slots = std::stoi(row["slots"]);

                   armors_.Add(armor, external_id);

                   ArmorAddon addon;
                   addon.name[ENGLISH] = row["en"];
                   addon.name[CHINESE] = row["zh"];
                   addon.name[JAPANESE] = row["jp"];

                   armor_addons_.Update(addon, external_id);
                   
                   return 0;
                 });

  // Armor Effects
  RunSQLiteQuery(database,
                 "SELECT Amr_ID AS external_id, "
                 "       SklTree_ID AS id, Pt AS points "
                 "FROM DB_SklTreetoAmr "
                 "ORDER BY Amr_ID;",
                 [](void *data, int argc, char **argv, char **caption) {
                   std::unordered_map<std::string, std::string> row;
                   GetRow(argc, argv, caption, &row);

                   int external_id = std::stoi(row["external_id"]);
                   Armor &armor = armors_.Select(external_id);
                   armor.effects.emplace_back();
                   armor.effects.back().id = skills_.Internalize(
                       std::stoi(row["id"]));
                   armor.effects.back().points = std::stoi(row["points"]);
                   return 0;
                 });

  // Armor Material
  RunSQLiteQuery(database,
                 "SELECT Amr_ID AS external_id, "
                 "       Itm_ID AS id "
                 "FROM DB_ItmtoAmr "
                 "ORDER BY Amr_ID;",
                 [](void *data, int argc, char **argv, char **caption) {
                   std::unordered_map<std::string, std::string> row;
                   GetRow(argc, argv, caption, &row);

                   int external_id = std::stoi(row["external_id"]);
                   ArmorAddon &addon = armor_addons_.Select(external_id);
                   addon.material_ids.push_back(items_.Internalize(
                       std::stoi(row["id"])));
                   
                   return 0;
                 });



  
}

}  // namespace dataset

}  // namespace monster_avengers
