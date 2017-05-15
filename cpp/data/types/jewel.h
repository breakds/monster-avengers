#pragma once

#include <string>
#include "data/types/properties.h"

namespace monster_avengers {

struct Jewel {
  Jewel() : id(0), dex_id(0), name(), slots(0), effect_list() {}

  int id;
  int dex_id;
  LangText name;
  int slots;
  EffectList effect_list;
};

}  // namespace monster_avengers
