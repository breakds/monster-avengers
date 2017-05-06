#pragma once

#include "data/types/properties.h"

namespace monster_avengers {
namespace data {

struct Skill {
  Skill() : name(), points(0) {}

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

}  // namespace data
}  // namespace monster_avengers
