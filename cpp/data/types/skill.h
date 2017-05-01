#pragma once

namespace monster_avengers {
namespace data {

struct Skill {
  Skill() : name(), points(0) {}

  LangText name;
  int points;
};

struct SkillTree {
  SkillTree() : id(0), dex_id(0), positives(), negatives() {}

  int id;
  int dex_id;
  std::vector<Skill> positives;
  std::vector<Skill> negatives;
};

}  // namespace data
}  // namespace monster_avengers
