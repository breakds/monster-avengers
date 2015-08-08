#ifndef _MONSTER_AVENGERS_DATASET_CORE_SKILL_H_
#define _MONSTER_AVENGERS_DATASET_CORE_SKILL_H_

#include "base/properties.h"

namespace monster_avengers {

namespace dataset {

struct Skill {

  Skill() = default;

  int points;
};

struct SkillSystem {
  std::vector<Skill> skills;
};

}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_CORE_SKILL_H_
