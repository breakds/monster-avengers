#include "data/types/properties.h"

#include "data/utils/enum_string.h"
#include "gtest/gtest.h"

namespace monster_avengers {
namespace data {

TEST(RangeTest, TextTest) {
  EXPECT_EQ(L"Blade", EnumString<Range>::Text(BLADEMASTER));
  EXPECT_EQ(L"Gunner", EnumString<Range>::Text(GUNNER));
  EXPECT_EQ(L"Both", EnumString<Range>::Text(RANGE_BOTH));
}

TEST(PartTest, TextTest) {
  EXPECT_EQ(L"Body", EnumString<Part>::Text(BODY));
  EXPECT_EQ(L"Head", EnumString<Part>::Text(HEAD));
  EXPECT_EQ(L"Hands", EnumString<Part>::Text(HANDS));
  EXPECT_EQ(L"Waist", EnumString<Part>::Text(WAIST));
  EXPECT_EQ(L"Feet", EnumString<Part>::Text(FEET));
  EXPECT_EQ(L"Weapon", EnumString<Part>::Text(WEAPON));
  EXPECT_EQ(L"Amulet", EnumString<Part>::Text(AMULET));
}

}  // namespace data
}  // namespace monster_avengers
