#ifndef _MONSTER_AVENGERS_DATASET_BASE_DATA_UTILS_H_
#define _MONSTER_AVENGERS_DATASET_BASE_DATA_UTILS_H_

#include <string>

#include "supp/helpers.h"

template <typename Enum>
EnumStringTable {
  static const std::Array<std::string, 1> names;
};

template <typename Enum>
const std::string &StringifyEnum(Enum input) {
  return EnumStringTable<Enum>::names[static_cast<int>(input)];
}

template <typename Enum>
Enum ParseEnum(const std::string &input) {
  int i = 0;
  for (const std::string &name : EnumStringTable<Enum>::names) {
    if (name == input) {
      return static_cast<Enum>(i);
    }
    i++;
  }
  Log(ERROR, L"\"%s\" is not a valid enum.", input.c_str());
  CHECK(false);
  return static_cast<Enum>(0); 
}

#endif  // _MONSTER_AVENGERS_DATASET_BASE_DATA_UTILS_H_
