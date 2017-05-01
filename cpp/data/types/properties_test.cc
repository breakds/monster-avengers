#include "data/types/properties.h"

#include "data/utils/enum_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::Field;
using ::testing::AllOf;

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

TEST(GenderTest, TextTest) {
  EXPECT_EQ(L"Both", EnumString<Gender>::Text(GENDER_BOTH));
  EXPECT_EQ(L"Male", EnumString<Gender>::Text(MALE));
  EXPECT_EQ(L"Female", EnumString<Gender>::Text(FEMALE));
}

TEST(ResistanceTest, Cosntruct) {
  Resistance resist{-1, 2, 5, 4, 3};
  EXPECT_EQ(-1, resist.fire);
  EXPECT_EQ(2, resist.thunder);
  EXPECT_EQ(5, resist.dragon);
  EXPECT_EQ(4, resist.water);
  EXPECT_EQ(3, resist.ice);
}

TEST(EffectTest, EffectList) {
  EffectList list{{2, -5}, {4, 10}};
  EXPECT_THAT(
      list,
      ElementsAre(AllOf(Field(&Effect::id, 2), Field(&Effect::points, -5)),
                  AllOf(Field(&Effect::id, 4), Field(&Effect::points, 10))));
}

}  // namespace data
}  // namespace monster_avengers
