#ifndef _MONSTER_AVENGERS_DATASET_CORE_ARMOR_H_
#define _MONSTER_AVENGERS_DATASET_CORE_ARMOR_H_

#include "base/properties.h"

namespace monster_avengers {

namespace dataset {

struct Armor {

  Armor() = default;

  ArmorPart part;
  WeaponType weapon_type;
  Gender gender;
  int rare;
  int min_defense;
  int max_defense;
  Resistance resistance;
  int slots;
  EffectList effects;
};

}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_CORE_ARMOR_H_
