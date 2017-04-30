#include "data/types/properties.h"

#include <string>

namespace monster_avengers {
namespace data {

const std::wstring &EnumString<WeaponType>::Text(WeaponType type) {
  static const std::wstring texts[] = {L"Melee", L"Range", L"Both"};
  return texts[type];
}

}  // namespace data
}  // namespace monster_avengers
