#pragma once

#include "data/utils/enum_string.h"

namespace monster_avengers {
namespace data {

enum Range {
  RANGE_UNKNOWN = -1,
  RANGE_BOTH = 0,
  BLADEMASTER = 1,
  GUNNER = 2,
};

template <>
struct EnumString<Range> {
  static const std::wstring &Text(Range type);
};

enum Part {
  PART_UNKNOWN = -1,
  BODY = 0,
  HEAD = 1,
  HANDS = 2,
  WAIST = 3,
  FEET = 4,
  WEAPON = 5,
  AMULET = 6,
};

template <>
struct EnumString<Part> {
  static const std::wstring &Text(Part part);
};

}  // namespace data
}  // namespace monster_avengers
