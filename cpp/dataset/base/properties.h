#ifndef _MONSTER_AVENGERS_DATASET_BASE_PROPERTIES_H_
#define _MONSTER_AVENGERS_DATASET_BASE_PROPERTIES_H_

#include <string>
#include "base/data_utils.h"

namespace monster_avengers {
  
namespace dataset {

// Enum WeaponType
enum WeaponType {
  WEAPON_TYPE_MELEE = 0,
  WEAPON_TYPE_RANGE = 1,
  WEAPON_TYPE_BOTH = 2,
};

template <>
struct EnumStringTable<WeaponType> {
  static const std::array<std::string, 3> names;
};
const std::array<std::string, 3> EnumStringTable<WeaponType>::names =
{"melee", "range", "both"};


// Enum ArmorPart
enum ArmorPart {
  HEAD = 0,
  HANDS = 1,
  WAIST = 2,
  FEET = 3,
  GEAR = 4,
  AMULET = 5,
  BODY = 6,
  PART_NUM
};

template <>
struct EnumStringTable<ArmorPart> {
  static const std::array<std::string, 7> names;
};
const std::array<std::string, 7> EnumStringTable<ArmorPart>::names =
{"head", "hands", "waist", "feet", "gear", "amulet", "body"};


// Enum Gender
enum Gender {
  GENDER_MALE = 0,
  GENDER_FEMALE = 1,
  GENDER_BOTH = 2,
};

template <>
struct EnumStringTable<Gender> {
  static const std::array<std::string, 3> names;
};
const std::array<std::string, 3> EnumStringTable<Gender>::names =
{"male", "female", "both"};




// Resistence is a struct of five integers representing the
// degrees to which the buff (debuff) is towards fire. thunder,
// dragon, water and ice, respectively.
struct Resistence {

  Resistence() = default;
      
  int fire;
  int thunder;
  int dragon;
  int water;
  int ice;
};

}  // namespace dataset

}  // monster_avengers

#endif  // _MONSTER_AVENGERS_DATASET_BASE_PROPERTIES_H_
