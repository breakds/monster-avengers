#ifndef _MONSTER_AVENGERS_DATASET_CORE_ARMOR_SET_H_
#define _MONSTER_AVENGERS_DATASET_CORE_ARMOR_SET_H_

#include <array>
#include "base/properties.h"

namespace monster_avengers {

namespace dataset {

typedef std::vector<int> JewelSet;

struct ArmorSet {
  ArmorSet() = default;
  std::array<int, PART_NUM> ids;
  std::array<JewelSet, PART_NUM> jewels;
};

}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_CORE_ARMOR_SET_H_
