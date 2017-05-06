#pragma once

#include <unordered_map>
#include <vector>
#include "data/types/armor.h"
#include "data/types/jewel.h"
#include "data/types/skill.h"

namespace monster_avengers {
namespace data {

class DexTranslator {
 public:
  DexTranslator() : to_dex_(), to_id_() {}

  int ToDex(int id) const {
    // TODO(breakds): Check existence.
    return to_dex_.at(id);
  }

  int FromDex(int dex_id) const {
    // TODO(breakds): Check existence.
    return to_id_.at(dex_id);
  }

  void Update(int id, int dex_id) {
    to_dex_[id] = dex_id;
    to_id_[dex_id] = id;
  }

 private:
  std::unordered_map<int, int> to_dex_;
  std::unordered_map<int, int> to_id_;
};

struct Data {
  Data()
      : armors(),
        armor_dex(),
        jewels(),
        jewel_dex(),
        skill_trees(),
        skill_tree_dex() {}

  std::vector<Armor> armors;
  DexTranslator armor_dex;

  std::vector<Jewel> jewels;
  DexTranslator jewel_dex;

  std::vector<SkillTree> skill_trees;
  DexTranslator skill_tree_dex;
};

}  // namespace data
}  // namespace monster_avengers
