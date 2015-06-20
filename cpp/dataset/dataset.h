#ifndef _MONSTER_AVENGERS_DATASET_DATASET_H_
#define _MONSTER_AVENGERS_DATASET_DATASET_H_

#include <array>
#include <string>

#include "addon/addons.h"
#include "base/data_utils.h"
#include "base/reindexed_table.h"
#include "base/properties.h"
#include "core/armor.h"
#include "core/armor_set.h"
#include "core/jewel.h"
#include "core/skill.h"


namespace monster_avengers {

namespace dataset {

class Arsenal;

class Data {
 public:
  inline static const Armor &armor(int internal_id) {
    return armors_[internal_id];
  }

  inline static const ReindexedTable<Armor> &armors() {
    return armors_;
  }

  inline static const Jewel &jewel(int internal_id) {
    return jewels_[internal_id];
  }

  inline static const ReindexedTable<Jewel> &jewels() {
    return jewels_;
  }

  inline static const SkillSystem &skill(int internal_id) {
    return skills_[internal_id];
  }

  inline static const ReindexedTable<SkillSystem> &skills() {
    return skills_;
  }

  static double EffectScore(const Effect &effect);
  
  // Data Loader 
  static void LoadBinary(const std::string &spec);
  static void LoadSQLite(const std::string &spec);

  // Data Services
  static void PrintSkill(int id, int verbose = 0, Language language = CHINESE);
  static void PrintJewel(int id, int verbose = 0, Language language = CHINESE);
  static void PrintArmor(const Armor &armor, int id, int verbose = 0,
                         Language language = CHINESE);
  static void PrintArmor(int id, int verbose = 0, Language language = CHINESE);
  static int GetMultiplier(const ArmorSet &armor_set, const Arsenal &arsenal);
  static std::vector<Effect> GetSkillStats(
      const ArmorSet &armor_set, const Arsenal &arsenal);
  static void PrintArmorSet(const ArmorSet &armor_set,
                            const Arsenal &arsenal,
                            int verbose = 0,
                            Language language = CHINESE);

 private:
  // Core Data
  static ReindexedTable<Armor> armors_;
  static ReindexedTable<Jewel> jewels_;
  static ReindexedTable<SkillSystem> skills_;

  // Addon Data
  static ReindexedTable<ArmorAddon> armor_addons_;
  static ReindexedTable<JewelAddon> jewel_addons_;
  static ReindexedTable<SkillSystemAddon> skill_addons_;
  static ReindexedTable<Item> items_;

  // Metadata
  static std::array<std::vector<int>, PART_NUM> armor_by_parts_;

  static void AddPredefinedArmors();
  static void ClassifyParts();
};

class Arsenal {
 public:
  Arsenal() = default;
  
  inline const Armor &operator[](int id) const {
    if (id >= Data::armors().size()) {
      return armors_[id - Data::armors().size()];
    }
    return Data::armor(id);
  }

  inline int size() const {
    return Data::armors().size() + static_cast<int>(armors_.size());
  }

  inline void clear() {
    armors_.clear();
  }

  inline void AddArmor(const Armor &armor) {
    armors_.push_back(armor);
  }

  static bool IsTorsoUp(const Armor &armor) {
    return 1 == armor.effects.size() && 0 == armor.effects[0].id;
  }
  
  inline bool IsTorsoUp(int id) const {
    return IsTorsoUp((operator[])(id));
  }

  inline void PrintArmor(int id, int verbose = 0,
                         Language language = CHINESE) {
    if (id < Data::armors().size()) {
      Data::PrintArmor(id, verbose, language);
    } else {
      Data::PrintArmor((operator[])(id),
                       id, verbose, language);
    }
  }

 private:
  std::vector<Armor> armors_;

  // TODO(breakds): remove this
  static std::array<std::vector<int>, PART_NUM> armor_by_parts_;  
};

}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_DATASET_H_
