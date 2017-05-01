#pragma once

#include "data/types/properties.h"

namespace monster_avengers {
namespace data {

struct Armor {
  Armor()
      : id(0),
        dex_id(0),
        part(PART_UNKNOWN),
        name(),
        range(RANGE_UNKNOWN),
        gender(GENDER_UNKNOWN),
        slots(0),
        rare(1),
        min_defense(0),
        max_defense(0),
        resist(),
        effect_list() {}

  int id;
  int dex_id;
  Part part;
  LangText name;
  Range range;
  Gender gender;
  int slots;
  int rare;
  int min_defense;
  int max_defense;
  Resistance resist;
  EffectList effect_list;
};

}  // namespace data
}  // namespace monster_avengers
