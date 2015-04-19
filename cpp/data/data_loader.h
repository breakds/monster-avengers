#ifndef _MONSTER_AVENGERS_DATA_LOADER_
#define _MONSTER_AVENGERS_DATA_LOADER_

#include <string>
#include <memory>
#include <vector>
#if _WIN32
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include <unordered_map>

#include "supp/helpers.h"
#include "lisp/lisp_object.h"
#include "lisp/reader.h"

#include "language_text.h"
#include "effect.h"
#include "skill.h"
#include "jewel.h"
#include "item.h"
#include "armor.h"

namespace monster_avengers {
  class LoaderCore {
  public:
    virtual ~LoaderCore() {}
    virtual std::vector<SkillSystem> LoadSkillSystems() const = 0;
    virtual std::vector<Jewel> LoadJewels() const = 0;
    virtual std::vector<Item> LoadItems() const = 0;
    virtual std::vector<Armor> LoadArmors() const = 0;
  };


  // ---------- Standard Loader Core ----------
  class StandardLoaderCore : public LoaderCore {
  public:
    StandardLoaderCore(const std::string &data_folder) 
      : data_folder_(data_folder) {}

    virtual std::vector<SkillSystem> LoadSkillSystems() const override {
      return lisp::ReadList<SkillSystem>(data_folder_ + "/skills.lisp");
    }

    virtual std::vector<Jewel> LoadJewels() const override {
      return lisp::ReadList<Jewel>(data_folder_ + "/jewels.lisp");
    }

    virtual std::vector<Item> LoadItems() const override {
      return lisp::ReadList<Item>(data_folder_ + "/items.lisp");
    }

    virtual std::vector<Armor> LoadArmors() const override {
      return lisp::ReadList<Armor>(data_folder_ + "/armors.lisp");
    }

  private:
    const std::string data_folder_;
  };

  
  // ---------- Dex SQLite Loader Core ----------
  void ToWstring(const std::string &input,
                 std::wstring *output) {
#if _WIN32
	wchar_t buffer[255];
#else
    wchar_t buffer[input.size()];
#endif
    swprintf(buffer, input.size(), L"%s", input.c_str());
    *output = buffer;
  }

  template <typename ObjectType>
  struct DexCallback {
    static int Do(std::vector<ObjectType> *store, 
                  const std::unordered_map<std::string, std::string> &row) {
      for (const auto &key_value : row) {
        wprintf(L"%s: %s\n", 
                key_value.first.c_str(), 
                key_value.second.c_str());
      }
      wprintf(L"\n");
      return 0;
    }
  };

  template <>
  struct DexCallback<SkillSystem> {
    static int Do(std::vector<SkillSystem> *store, 
                  const std::unordered_map<std::string, std::string> &row) {
      int id = std::stoi(row.at("SklTree_ID")) - 1;
      if (store->size() - 1 == id) {
        LanguageText name;
        ToWstring(row.at("Skl_Name_0"), &name.en);
        ToWstring(row.at("Skl_Name_3"), &name.jp);
        store->back().skills.emplace_back(std::stoi(row.at("Pt")), 
                                         LanguageText());
      } else if (store->size() == id) {
        LanguageText name;
        ToWstring(row.at("SklTree_Name_0"), &name.en);
        ToWstring(row.at("SklTree_Name_3"), &name.jp);
        store->emplace_back(name, id);
        ToWstring(row.at("Skl_Name_0"), &name.en);
        ToWstring(row.at("Skl_Name_3"), &name.jp);
        store->back().skills.emplace_back(std::stoi(row.at("Pt")),
                                          name);
      } else {
        CHECK(false);
      }
      return 0;
    }
  };

  template <>
  struct DexCallback<Jewel> {
    static int Do(std::vector<Jewel> *store, 
                  const std::unordered_map<std::string, std::string> &row) {
      store->emplace_back();
      LanguageText name;
      ToWstring(row.at("Itm_Name_0"), &name.en);
      ToWstring(row.at("Itm_Name_3"), &name.jp);
      store->back().name = std::move(name);
      store->back().holes = std::stoi(row.at("Slot"));
      store->back().external_id = std::stoi(row.at("Jew_ID"));
      int skill_id = std::stoi(row.at("SklTree1_ID"));
      if (skill_id > -1) {
        store->back().effects.emplace_back(skill_id - 1,
                                           std::stoi(row.at("SklTree1_Pt")));
      }
      skill_id = std::stoi(row.at("SklTree2_ID"));
      if (skill_id > -1) {
        store->back().effects.emplace_back(skill_id - 1,
                                           std::stoi(row.at("SklTree2_Pt")));
      }
      return 0;
    }
  };

  template <>
  struct DexCallback<Item> {
    static int Do(std::vector<Item> *store, 
                  const std::unordered_map<std::string, std::string> &row) {
      int id = std::stoi(row.at("Itm_ID"));
      CHECK(id - 1 == store->size());
      store->emplace_back();
      ToWstring(row.at("Itm_Name_0"), &store->back().name.en);
      ToWstring(row.at("Itm_Name_3"), &store->back().name.jp);
      return 0;
    }
  };

  template <>
  struct DexCallback<Armor> {
    static int Do(std::vector<Armor> *store, 
                  const std::unordered_map<std::string, std::string> &row) {
      int id = std::stoi(row.at("Amr_ID")) - 1;
      if (store->size() - 1 == id) {
        store->back().effects.emplace_back(
            std::stoi(row.at("SklTree_ID")) - 1,
            std::stoi(row.at("Pt")));
      } else if (store->size() == id) {
        store->emplace_back();

        LanguageText name;
        ToWstring(row.at("Amr_Name_0"), &store->back().name.en);
        ToWstring(row.at("Amr_Name_3"), &store->back().name.jp);

        switch (std::stoi(row.at("Part"))) {
          case 1:
            store->back().part = HEAD;
            break;
          case 2:
            store->back().part = BODY;
            break;
          case 3:
            store->back().part = HANDS;
            break;
          case 4:
            store->back().part = WAIST;
            break;
          case 5:
            store->back().part = FEET;
            break;
        }

        switch (std::stoi(row.at("MorF"))) {
          case 0:
            store->back().gender = BOTH_GENDER;
            break;
          case 1:
            store->back().gender = MALE;
            break;
          case 2:
            store->back().gender = FEMALE;
            break;
        }

        store->back().rare = std::stoi(row.at("Rare"));

        store->back().min_defense = 0;
        store->back().max_defense = std::stoi(row.at("MaxDef"));

        store->back().resistence.fire = std::stoi(row.at("Res_Fire"));
        store->back().resistence.thunder = std::stoi(row.at("Res_Thunder"));
        store->back().resistence.dragon = std::stoi(row.at("Res_Dragon"));
        store->back().resistence.water = std::stoi(row.at("Res_Water"));
        store->back().resistence.ice = std::stoi(row.at("Res_Ice"));

        store->back().holes = std::stoi(row.at("Slot"));

        store->back().external_id = id + 1;

        if ("NULL" != row.at("SklTree_ID")) {
          store->back().effects.emplace_back(
              std::stoi(row.at("SklTree_ID")) - 1,
              std::stoi(row.at("Pt")));
        }
      } else {
        CHECK(false);
      }
      return 0;
    }
  };

  template <typename ObjectType>
  struct DexCallbackWrapper {
    static int Do(void *data, int argc, char **argv, char **captions) {
      std::vector<ObjectType> *store = 
        static_cast<std::vector<ObjectType>*>(data);

      std::unordered_map<std::string, std::string> row;
      for (int i = 0; i < argc; ++i) {
        row[captions[i]] = argv[i] ? argv[i] : "NULL";
      }
      return DexCallback<ObjectType>::Do(store, row);
    }
  };

  class DexSqliteLoaderCore : public LoaderCore {
  public:
    DexSqliteLoaderCore(const std::string &path_specifier) {
      CHECK(!sqlite3_open(path_specifier.c_str(), &database_));
    }

    virtual ~DexSqliteLoaderCore() {
      sqlite3_close(database_);
    }

    virtual std::vector<SkillSystem> LoadSkillSystems() const override {
      return Load<SkillSystem>(
          "SELECT ID_SklTree_Name.SklTree_ID, SklTree_Name_0, "
          "SklTree_Name_3, Skl_Name_0, Skl_Name_3, Pt "
          "FROM ID_SklTree_Name "
          "INNER JOIN DB_Skl ON "
          "ID_SklTree_Name.SklTree_ID = DB_Skl.SklTree_ID "
          "INNER JOIN ID_Skl_Name ON "
          "ID_Skl_Name.Skl_ID = DB_Skl.Skl_ID "
          "ORDER BY ID_SklTree_Name.SklTree_ID;");
    }
    
    virtual std::vector<Jewel> LoadJewels() const override {
      return Load<Jewel>(
          "SELECT Jew_ID, DB_Jew.Itm_ID, "
          "       Itm_Name_0, Itm_Name_3, Slot, "
          "       SklTree1_ID, SklTree1_Pt, "
          "       SklTree2_ID, SklTree2_Pt "
          "FROM DB_Jew "
          "INNER JOIN ID_Itm_Name ON "
          "ID_Itm_Name.Itm_ID = DB_Jew.Itm_ID "
          "WHERE DB_Jew.Itm_ID < 1774 "
          "GROUP BY DB_Jew.Itm_ID "
          "ORDER BY Itm_Name_0");
    }

    virtual std::vector<Item> LoadItems() const override {
      return Load<Item>("SELECT Itm_ID, Itm_Name_0, Itm_Name_3 "
                        "FROM ID_Itm_Name "
                        "WHERE Itm_ID > 0 "
                        "ORDER BY Itm_ID;");
    }

    virtual std::vector<Armor> LoadArmors() const override {
      std::vector<Armor> result = std::move(Load<Armor>(
          "SELECT DB_Amr.Amr_ID, ID_Amr_Name.Amr_Name_0, "
          "ID_Amr_Name.Amr_Name_3, Part, Slot, Rare, "
          "BorG, MorF, Def, MaxDef, Res_Fire, "
          "Res_Thunder, Res_Dragon, Res_Water, "
          "Res_Ice, SklTree_ID, Pt "
          "FROM DB_Amr "
          "INNER JOIN ID_Amr_Name ON "
          "ID_Amr_Name.Amr_ID = DB_Amr.Amr_ID "
          "LEFT JOIN DB_SklTreetoAmr ON "
          "DB_SklTreetoAmr.Amr_ID = DB_Amr.Amr_ID "
          "ORDER BY DB_Amr.Amr_ID;"));

      for (int slot = 0; slot < 4; slot++) {
        result.emplace_back();
        result.back().name.en = L"Weapon";
        result.back().part = GEAR;
        result.back().external_id = -1;
        result.back().gender = BOTH_GENDER;
        result.back().type = BOTH;
        result.back().holes = slot;
        result.back().rare = 10;
      }

      return result;
    }

  private:
    template <typename ObjectType>
    std::vector<ObjectType> Load(const std::string &query) const {
      std::vector<ObjectType> result;
      char *error_message = nullptr;
      int status = sqlite3_exec(database_, query.c_str(),
                                DexCallbackWrapper<ObjectType>::Do,
                                &result, &error_message);
      if (SQLITE_OK != status) {
        Log(FATAL, L"SQLite Query Error: %s\n", error_message);
        sqlite3_free(error_message);
        CHECK(false);
        return {};
      }
      return result;
    }
    
    sqlite3 *database_;
  };
  
  
  // ---------- Interface: DataLoader ----------
  class DataLoader {
  public:
    DataLoader() : core_(nullptr) {}
    
    void Initialize(const std::string &descriptor) {
      std::string format;
      std::string path;
      CHECK(ParseDescriptor(descriptor, &format, &path));
      if ("standard" == format) {
        core_.reset(new StandardLoaderCore(path));
      } else if ("dex" == format) {
        core_.reset(new DexSqliteLoaderCore(path));
      } else {
        CHECK(false);
      }
    }

    const LoaderCore *const Get() {
      return core_.get();
    }

  private:
    // Separate a path descriptor into two parts, the prefix
    // (format) and the postfix (the actual path). The delimiter is
    // ':'.
    static bool ParseDescriptor(const std::string &descriptor, 
                                std::string *prefix,
                                std::string *postfix) {
      size_t colon_index = descriptor.find_first_of(':');
      if (colon_index == std::string::npos) {
        // Fallback to standard case when we cannot find format
        // specifier in descriptor.
        *prefix = "standard";
        *postfix = descriptor;
        return true;
      }
      *prefix = descriptor.substr(0, colon_index);
      *postfix = descriptor.substr(colon_index + 1);
      return true;
    }

    std::unique_ptr<LoaderCore> core_;
  };
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_DATA_LOADER_
