#pragma once

#include <vector>
#include "data/utils/enum_string.h"

namespace monster_avengers {

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

enum Gender {
  GENDER_UNKNOWN = -1,
  GENDER_BOTH = 0,
  MALE = 1,
  FEMALE = 2,
};

template <>
struct EnumString<Gender> {
  static const std::wstring &Text(Gender gender);
};

// Resistance is a struct of five integers representing the degrees to
// which the buff (debuff) is towards fire. thunder, dragon, water and
// ice, respectively.
struct Resistance {
  Resistance() = default;

  int fire;
  int thunder;
  int dragon;
  int water;
  int ice;
};

// Effect is a pair of integer. It represents the point contribution
// towards a the id-specified skill tree.
struct Effect {
  Effect() : id(-1), points(0) {}
  Effect(int input_id, int input_points) : id(input_id), points(input_points) {}

  int id;
  int points;
};

using EffectList = std::vector<Effect>;
using SkillTreeIdList = std::vector<int>;

// LangText is a text with i18ns.
struct LangText {
  LangText() = default;

  std::wstring eng;
  std::wstring chs;
  std::wstring jpn;
};

}  // namespace monster_avengers
