#pragma once

#include "data/types/armor.h"
#include "data/types/jewel.h"
#include "data/types/skill.h"

namespace monster_avengers {
namespace data {

struct Data {
  std::vector<Armor> armors;
  std::vector<Jewel> jewels;
  std::vector<SkillTree> skill_trees;
};

}  // namespace data
}  // namespace monster_avengers
