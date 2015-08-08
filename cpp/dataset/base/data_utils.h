#ifndef _MONSTER_AVENGERS_DATASET_BASE_DATA_UTILS_H_
#define _MONSTER_AVENGERS_DATASET_BASE_DATA_UTILS_H_

#include <string>
#include <array>

#include "supp/helpers.h"

namespace monster_avengers {

namespace dataset {

template <typename Enum>
struct EnumStringTable {
  static const std::array<std::string, 1> names;
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
  Log(FATAL, L"\"%s\" is not a valid enum.", input.c_str());
  CHECK(false);
  return static_cast<Enum>(0); 
}

}  // namesapce dataset 

}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_DATASET_BASE_DATA_UTILS_H_
