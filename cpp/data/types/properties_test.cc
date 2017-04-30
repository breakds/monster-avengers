#include "data/types/properties.h"

#include "gtest/gtest.h"
#include "data/utils/enum_string.h"

namespace monster_avengers {
namespace data {

TEST(WeaponTypeTest, TextTest) {
  EXPECT_EQ(L"Melee", EnumString<WeaponType>::Text(WEAPON_MELEE));
}

}  // namespace data
}  // namespace monster_avengers
