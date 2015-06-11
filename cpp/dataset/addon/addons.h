#ifndef _MONSTER_AVENGERS_DATASET_ADDON_ADDONS_H_
#define _MONSTER_AVENGERS_DATASET_ADDON_ADDONS_H_

#include "base/properties.h"

namespace monster_avengers {

namespace dataset {

struct ArmorAddon {

  ArmorAddon() = default;

  LangText name;
  std::vector<int> material_ids;
};


struct JewelAddon {

  JewelAddon() = default;
  
  LangText name;
};

struct SkillSystemAddon {
  SkillSystemAddon() = default;
  
  LangText name;
  std::vector<LangText> skill_names;
};

struct Item {
  Item() = default;
  LangText name;
};

}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_ADDON_ADDONS_H_
