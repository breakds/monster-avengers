#ifndef _MONSTER_AVENGERS_DATA_SET_
#define _MONSTER_AVENGERS_DATA_SET_

#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>
#include <map>
#include <unordered_map>

#include "aux/helpers.h"
#include "lisp/lisp_object.h"
#include "lisp/reader.h"

#include "language_text.h"
#include "effect.h"
#include "skill.h"
#include "jewel.h"
#include "item.h"
#include "armor.h"

namespace monster_avengers {
  
  class DataSet {
  public:
    int torso_up_id;
    
    DataSet(const std::string &data_folder) 
      : skill_systems_(), jewels_(), armors_(),
        armor_indices_by_parts_() {
      // ---------- Skills ----------
      // TROSO UP is alwasy the skill system with id 0.
      torso_up_id = 0; 
      skill_systems_ = lisp::ReadList<SkillSystem>(data_folder + 
                                                   "/skills.lisp");

      // ---------- Jewels ----------
      jewels_ = lisp::ReadList<Jewel>(data_folder + "/jewels.lisp");
      
      // ---------- Items ----------
      items_ = lisp::ReadList<Item>(data_folder + "/items.lisp");


      // ---------- Armors ----------
      {
        armors_ = lisp::ReadList<Armor>(data_folder + "/armors.lisp");
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

    inline const LanguageText &ItemName(int id) const {
      return items_[id].name;
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

#endif  // _MONSTER_AVENGERS_DATA_SET_
