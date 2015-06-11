#include "base/properties.h"

namespace monster_avengers {
  
namespace dataset {

const std::array<std::string, 3> EnumStringTable<WeaponType>::names =
{"melee", "range", "both"};

const std::array<std::string, 7> EnumStringTable<ArmorPart>::names =
{"head", "hands", "waist", "feet", "gear", "amulet", "body"};

const std::array<std::string, 3> EnumStringTable<Gender>::names =
{"male", "female", "both"};

const std::array<std::string, LANGUAGE_NUM> EnumStringTable<Language>::names =
{"en", "zh", "jp"};

}  // namespace dataset

}  // monster_avengers

