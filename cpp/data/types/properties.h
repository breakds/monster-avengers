#pragma once

#include "data/utils/enum_string.h"

namespace monster_avengers {
namespace data {

enum WeaponType {
  WEAPON_MELEE = 0,
  WEAPON_RANGE = 1,
  WEAPON_BOTH = 2,
};

template <>
struct EnumString<WeaponType> {
  static const std::wstring &Text(WeaponType type);
};

}  // namespace data
}  // namespace monster_avengers
