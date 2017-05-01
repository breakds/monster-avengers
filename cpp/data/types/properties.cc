#include "data/types/properties.h"

#include <string>

namespace monster_avengers {
namespace data {

const std::wstring &EnumString<Range>::Text(Range type) {
  static const std::wstring texts[] = {L"Both", L"Blade", L"Gunner"};
  return texts[type];
}

const std::wstring &EnumString<Part>::Text(Part part) {
  static const std::wstring texts[] = {L"Body", L"Head",   L"Hands", L"Waist",
                                       L"Feet", L"Weapon", L"Amulet"};
  return texts[part];
}

const std::wstring &EnumString<Gender>::Text(Gender type) {
  static const std::wstring texts[] = {L"Both", L"Male", L"Female"};
  return texts[type];
}

}  // namespace data
}  // namespace monster_avengers
