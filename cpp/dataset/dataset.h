#ifndef _MONSTER_AVENGERS_DATASET_DATASET_H_
#define _MONSTER_AVENGERS_DATASET_DATASET_H_

#include <string>

#include "addon/addons.h"
#include "base/data_utils.h"
#include "base/reindexed_table.h"
#include "base/properties.h"
#include "core/armor.h"
#include "core/jewel.h"
#include "core/skill.h"


namespace monster_avengers {

namespace dataset {

class Data {
 public:
  static void Initialize(const std::string &spec);

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
};

}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_DATASET_H_
