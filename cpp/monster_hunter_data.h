#ifndef _MONSTER_AVENGERS_MONSTER_HUNTER_DATA_
#define _MONSTER_AVENGERS_MONSTER_HUNTER_DATA_

#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>
#include <map>
#include <unordered_map>

#include "helpers.h"
#include "parser.h"


namespace monster_avengers {

  struct LanguageText {
    std::wstring en;
    std::wstring jp;

    LanguageText() : en(L""), jp(L"") {}

    LanguageText(parser::Tokenizer *tokenizer, 
                 bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"JP" == token.value) {
            jp = tokenizer->ExpectString();
          } else if (L"EN" == token.value) {
            en = tokenizer->ExpectString();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    }

    LanguageText(LanguageText &&other) {
      en.swap(other.en);
      jp.swap(other.jp);
    }

    const LanguageText &operator=(LanguageText &&other) {
      en.swap(other.en);
      jp.swap(other.jp);
      return *this;
    }

    const wchar_t *c_str() const {
      return jp.c_str();
    }
  };

  struct Skill {
    int points;
    LanguageText name;
    LanguageText description;

    // Constructor from reading Tokenizer.
    Skill(parser::Tokenizer *tokenizer, 
          bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"DESCRIPTION" == token.value) {
            description = LanguageText(tokenizer);
          } else if (L"NAME" == token.value) {
            name = LanguageText(tokenizer);
          } else if (L"POINTS" == token.value) {
            points = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    } 

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Skill {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.jp.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"description: %ls\n", description.en.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"points: %d\n", points);
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };

  struct SkillSystem {
    LanguageText name;
    std::vector<Skill> skills;
    int id;

    // Constructor from reading Tokenizer.
    explicit SkillSystem(parser::Tokenizer *tokenizer,
                         bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;

      while (!complete) {
        CHECK(tokenizer->Next(&token));
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"NAME" == token.value) {
            name = LanguageText(tokenizer);
          } else if (L"SKILLS" == token.value) {
            skills = std::move(parser::ParseList<Skill>::Do(tokenizer));
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else if (L"ID" == token.value) {
            id = tokenizer->ExpectNumber();
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"SkillSystem {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.jp.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"skills: [\n");
      for (const Skill &skill : skills) {
        skill.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
  
  struct Effect {
    int skill_id;
    int points;
    
    // Constructor from reading Tokenizer.
    Effect(parser::Tokenizer *tokenizer, 
           bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"SKILL-ID" == token.value) {
            skill_id = tokenizer->ExpectNumber();
          } else if (L"POINTS" == token.value) {
            points = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    } 

    Effect(int input_skill_id, int input_points) 
      : skill_id(input_skill_id), points(input_points) {}

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Effect {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"skill_id: %d\n", skill_id);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"points: %d\n", points);
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
  
  struct Jewel {
    LanguageText name;
    int holes;
    std::vector<Effect> effects;
    
    // Constructor from reading Tokenizer.
    explicit Jewel(parser::Tokenizer *tokenizer,
                   bool expect_open_paren = true) 
      : effects() {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;

      while (!complete) {
        CHECK(tokenizer->Next(&token));
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"NAME" == token.value) {
            name = LanguageText(tokenizer);
          } else if (L"EFFECTS" == token.value) {
            effects = std::move(parser::ParseList<Effect>::Do(tokenizer));
          } else if (L"SLOTS" == token.value) {
            holes = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }

      auto it = std::remove_if(effects.begin(), effects.end(),
                               [](const Effect &effect) {
                                 return -1 == effect.skill_id;
                               });
      for (int i = 0; i < effects.end() - it; ++i) {
        effects.pop_back();
      }
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Jewel {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name (JP): %ls\n", name.jp.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name (EN): %ls\n", name.en.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"holes: %d\n", holes);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"effects: [\n");
      for (const Effect &effect : effects) {
        effect.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };

  enum WeaponType {
    MELEE = 0,
    RANGE,
    BOTH,
  };

  enum ArmorPart {
    HEAD = 0,
    BODY = 1,
    HANDS = 2,
    WAIST = 3,
    FEET = 4,
    GEAR = 5,
    AMULET = 6,
    PART_NUM
  };

  enum Gender {
    MALE = 0,
    FEMALE,
    BOTH_GENDER,
  };

  struct Resistence {
    int fire;
    int thunder;
    int dragon;
    int water;
    int ice;

    Resistence() : fire(0),
                   thunder(0),
                   dragon(0),
                   water(0),
                   ice(0) {}

    Resistence(parser::Tokenizer *tokenizer, 
               bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"FIRE" == token.value) {
            fire = tokenizer->ExpectNumber();
          } else if (L"THUNDER" == token.value) {
            thunder = tokenizer->ExpectNumber();
          } else if (L"DRAGON" == token.value) {
            dragon = tokenizer->ExpectNumber();
          } else if (L"WATER" == token.value) {
            water = tokenizer->ExpectNumber();
          } else if (L"ICE" == token.value) {
            ice = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    }
  };

  struct Item {
    LanguageText name;
    int id;
    
    Item(parser::Tokenizer *tokenizer,
         bool expect_open_paren = true) {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"NAME" == token.value) {
            name = LanguageText(tokenizer);
          } else if (L"ID" == token.value) {
            id = tokenizer->ExpectNumber();
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Item {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"%d: %ls  %ls\n", id,
              name.jp.c_str(), name.en.c_str());
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"}\n");
    }
  };

  struct Armor {
    LanguageText name;
    ArmorPart part;
    WeaponType type;
    Gender gender;
    int rare;
    int min_defense;
    int max_defense;
    Resistence resistence;
    int holes;
    std::vector<Effect> effects;
    std::vector<int> material;
    // Whether the armor is created only for being multiplied.
    bool multiplied;
    // What is the base armor, in torso up case.
    int base;
    // The stuffed jewels
    std::unordered_map<int, int> jewels;
    
    Armor() = default;

    static Armor Amulet(int holes, std::vector<Effect> effects) {
      Armor armor;
      armor.name.en = L"----";
      armor.name.jp = L"----";
      armor.part = AMULET;
      armor.type = BOTH;
      armor.gender = BOTH_GENDER;
      armor.rare = 10;
      armor.min_defense = 0;
      armor.max_defense = 0;
      armor.holes = holes;
      armor.effects = std::move(effects);
      armor.multiplied = false;
      armor.base = -1;
      armor.jewels.clear();
      return armor;
    }

    Armor(const Armor &other) {
      name.en = other.name.en;
      name.jp = other.name.jp;
      part = other.part;
      type = other.type;
      gender = other.gender;
      rare = other.rare;
      min_defense = other.min_defense;
      max_defense = other.max_defense;
      holes = other.holes;
      effects = other.effects;
      multiplied = other.multiplied;
      base = other.base;
      jewels = other.jewels;
    }

    // Constructor from reading Tokenizer.
    Armor(parser::Tokenizer *tokenizer,
          bool expect_open_paren = true) 
      : multiplied(false), base(-1), jewels() {
      if (expect_open_paren) {
        tokenizer->Expect(parser::OPEN_PARENTHESIS);
      }
      parser::Token token;
      bool complete = false;
      std::wstring tmp_string;
      
      
      while (!complete) {
        CHECK(tokenizer->Next(&token));
        switch (token.name) {
        case parser::CLOSE_PARENTHESIS:
          complete = true;
          break;
        case parser::KEYWORD:
          if (L"NAME" == token.value) {
            name = LanguageText(tokenizer);
          } else if (L"PART" == token.value) {
            tmp_string = tokenizer->ExpectString();
            if (L"Head" == tmp_string) {
              part = HEAD;
            } else if (L"Body" == tmp_string) {
              part = BODY;
            } else if (L"Arms" == tmp_string) {
              part = HANDS;
            } else if (L"Waist" == tmp_string) {
              part = WAIST;
            } else if (L"Legs" == tmp_string) {
              part = FEET;
            } else if (L"gear" == tmp_string) {
              part = GEAR;
            } else {
              Log(ERROR, L"Unrecognizable part: %ls", 
                  tmp_string.c_str());
              CHECK(false);
            }
          } else if (L"GENDER" == token.value) {
            tmp_string = tokenizer->ExpectString();
            if (L"Male" == tmp_string) {
              gender = MALE;
            } else if (L"FEMALE" == tmp_string) {
              gender = FEMALE;
            } else {
              gender = BOTH_GENDER;
            }
          } else if (L"TYPE" == token.value) {
            tmp_string = tokenizer->ExpectString();
            if (L"Blade" == tmp_string) {
              type = MELEE;
            } else if (L"Gunner" == tmp_string) {
              type = RANGE;
            } else {
              type = BOTH;
            }
          } else if (L"SLOTS" == token.value) {
            holes = tokenizer->ExpectNumber();
          } else if (L"RARE" == token.value) {
            rare = tokenizer->ExpectNumber();
          } else if (L"MIN-DEFENSE" == token.value) {
            min_defense = tokenizer->ExpectNumber();
          } else if (L"MAX-DEFENSE" == token.value) {
            max_defense = tokenizer->ExpectNumber();
          } else if (L"RESISTENCE" == token.value) {
            resistence = Resistence(tokenizer);
          } else if (L"MATERIAL" == token.value) { 
            material = std::move(parser::ParseList<int>::Do(tokenizer));
          } else if (L"EFFECTS" == token.value) {
            effects = std::move(parser::ParseList<Effect>::Do(tokenizer));
          } else if (L"OBJ" == token.value) {
            CHECK(tokenizer->Expect(parser::TRUE));
          } else {
            Log(ERROR, L"Unexpected keyword :%ls", token.value.c_str());
            CHECK(false);
          }
          break;
        default:
          // Shouldn't have entered here.
          CHECK(false);
        }
      }
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Armor {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"type: %ls\n", BOTH == type ? L"BOTH" 
              : (MELEE == type ? L"MELEE" : L"RANGE"));
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"defense: %d\n", max_defense);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"rare: %d\n", rare);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"holes: %d\n", holes);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"effects: [\n");
      for (const Effect &effect : effects) {
        effect.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
  
  class DataSet {
  public:
    int torso_up_id;
    
    DataSet(const std::string &data_folder) 
      : skill_systems_(), jewels_(), armors_(),
        armor_indices_by_parts_() {
      // ---------- Skills ----------
      // TROSO UP is alwasy the skill system with id 0.
      torso_up_id = 0; 
      {
        const std::string path = data_folder + "/skills.lisp";
        auto tokenizer = std::move(parser::Tokenizer::FromFile(path));
        skill_systems_ = 
          std::move(parser::ParseList<SkillSystem>::Do(&tokenizer));
      }

      // ---------- Jewels ----------
      {
        const std::string path = data_folder + "/jewels.lisp";
        auto tokenizer = std::move(parser::Tokenizer::FromFile(path));
        jewels_ = 
          std::move(parser::ParseList<Jewel>::Do(&tokenizer));
      }

      // ---------- Items ----------
      {
        const std::string path = data_folder + "/items.lisp";
        auto tokenizer = std::move(parser::Tokenizer::FromFile(path));
        items_ = 
          std::move(parser::ParseList<Item>::Do(&tokenizer));
      }

      // ---------- Armors ----------
      {
        const std::string path = data_folder + "/armors.lisp";
        auto tokenizer = std::move(parser::Tokenizer::FromFile(path));
        armors_ = std::move(parser::ParseList<Armor>::Do(&tokenizer));
        // Add the dummy amulet.
        armors_.push_back(Armor::Amulet(0, {}));
        // Initialize armor_indices_by_parts_
        armor_indices_by_parts_.resize(PART_NUM);
        int i = 0;
        for (const Armor &armor : armors_) {
          armor_indices_by_parts_[armor.part].push_back(i++);
        }
        reserved_armor_count_ = static_cast<int>(armors_.size());
      }
    }

    inline const std::vector<Jewel> &jewels() const {
      return jewels_;
    }

    inline const Jewel &jewel(int id) const {
      return jewels_[id];
    }

    inline const std::vector<int> &ArmorIds(ArmorPart part) const {
      return armor_indices_by_parts_[part];
    }

    inline const Armor &armor(int id) const {
      return armors_[id];
    }

    inline const Armor &armor(ArmorPart part, int id) const {
      return armors_[armor_indices_by_parts_[part][id]];
    }

    inline bool ProvidesTorsoUp(ArmorPart part, int id) const {
      const Armor &armor = armors_[armor_indices_by_parts_[part][id]];
      return  1 == armor.effects.size() &&
        armor.effects[0].skill_id == torso_up_id;
    }

    inline bool ProvidesTorsoUp(int id) const {
      const Armor &armor = armors_[id];
      return  1 == armor.effects.size() &&
        armor.effects[0].skill_id == torso_up_id;
    }

    inline const std::vector<Armor> &armors() const {
      return armors_;
    }

    inline const SkillSystem &skill_system(int id) const {
      return skill_systems_[id];
    }

    inline void AddExtraArmor(ArmorPart part, const Armor &armor) {
      armor_indices_by_parts_[part].push_back(armors_.size());
      armors_.push_back(armor);
    }

    inline void ClearExtraArmor() {
      while (armors_.size() > reserved_armor_count_) {
        armors_.pop_back();
      }
    }

    void PrintSkillSystems() {
      for (int i = 0; i < skill_systems_.size(); ++i) {
        wprintf(L"%d: %ls\n", i, skill_systems_[i].name.c_str());
      }
    }

    double EffectScore(const Effect &effect) {
      int armor_count = 0;
      for (int i = 0; i < reserved_armor_count_; ++i) {
        for (const Effect &armor_effect : armors_[i].effects) {
          if (armor_effect.skill_id == effect.skill_id) {
            armor_count++;
            break;
          }
        }
      }

      double jewel_index = 0;
      for (const Jewel &jewel : jewels_) {
        for (const Effect &jewel_effect : jewel.effects) {
          if (jewel_effect.skill_id == effect.skill_id) {
            jewel_index += 
              static_cast<double>(jewel_effect.points) / jewel.holes *
              std::exp(-jewel.holes * 0.1);

          }
        }
      }
      
      return std::exp(0.1 * jewel_index - 0.3 * effect.points)
        * armor_count;
    }

    void Summarize() {
      Log(INFO, L"Skill Systems: %lld", skill_systems_.size());
      Log(INFO, L"Jewels: %lld", jewels_.size());
      Log(INFO, L"Armors: %lld", armors_.size());
      Log(INFO, L" - HELMS: %lld", armor_indices_by_parts_[HEAD].size());
      Log(INFO, L" - CUIRASSES: %lld", armor_indices_by_parts_[BODY].size());
      Log(INFO, L" - GLOVES: %lld", armor_indices_by_parts_[HANDS].size());
      Log(INFO, L" - CUISSES: %lld", armor_indices_by_parts_[WAIST].size());
      Log(INFO, L" - SABATONS: %lld", armor_indices_by_parts_[FEET].size());
      Log(INFO, L" - GEARS: %lld", armor_indices_by_parts_[GEAR].size());
      Log(INFO, L" - AMULETS: %lld", armor_indices_by_parts_[AMULET].size());
    }
    
  private:
    std::vector<SkillSystem> skill_systems_;
    std::vector<Jewel> jewels_;
    std::vector<Item> items_;
    std::vector<Armor> armors_;
    int reserved_armor_count_;
    std::vector<std::vector<int> > armor_indices_by_parts_;
  };

}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_MONSTER_HUNTER_DATA_
