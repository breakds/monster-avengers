#pragma once

#include <algorithm>
#include <cstdio>
#include <cstring>
#include "data/types/properties.h"

namespace monster_avengers {

struct Signature {
  Signature() : values() {}

  Signature(const EffectList &effect_list, const SkillTreeIdList &ids)
      : values(ids.size()) {
    memset(&values[0], 0, values.size());
    for (const Effect &effect : effect_list) {
      auto iter = std::find(ids.begin(), ids.end(), effect.id);
      if (iter != ids.end()) {
        values[iter - ids.begin()] = effect.points;
      }
    }
  }

  // ---------- Properties ----------

  size_t size() {
    return values.size();
  }

  bool HasPositive() {
    for (const char &value : values) {
      if (value > 0) {
        return true;
      }
    }
    return false;
  }

  // ---------- Operators ----------

  std::vector<char> values;
};

}  // namespace monster_avengers
