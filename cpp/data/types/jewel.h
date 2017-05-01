#pragma once

#include "data/types/properties.h"

namespace monster_avengers {
namespace data {

struct Jewel {
  Jewel() : id(0), dex_id(0), name(), slots(0), effect_list() {}

  int id;
  int dex_id;
  LangText name;
  int slots;
  EffectList effect_list;
};

}  // namespace data
}  // namespace monster_avengers
