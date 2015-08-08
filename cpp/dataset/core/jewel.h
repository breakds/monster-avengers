#ifndef _MONSTER_AVENGERS_DATASET_CORE_JEWEL_H_
#define _MONSTER_AVENGERS_DATASET_CORE_JEWEL_H_

#include "base/properties.h"

namespace monster_avengers {

namespace dataset {

struct Jewel {

  Jewel() = default;

  int slots;
  EffectList effects;
};

}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_CORE_JEWEL_H_
