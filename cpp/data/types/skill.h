#pragma once

#include "data/types/properties.h"

namespace monster_avengers {

struct Skill {
  Skill() : name(), points(0) {}
  
  Skill(LangText &&input_name, int input_points)
      : name(input_name), points(input_points) {}

  LangText name;
  int points;
};

struct SkillTree {
  SkillTree() : id(0), dex_id(0), name(), positives(), negatives() {}

  int id;
  int dex_id;
  LangText name;
  std::vector<Skill> positives;
  std::vector<Skill> negatives;
};

}  // namespace monster_avengers
