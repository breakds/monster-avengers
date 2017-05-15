#include "data/types/armor.h"
#include "data/types/jewel.h"
#include "data/types/skill.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Field;

namespace monster_avengers {

TEST(ArmorTest, DefaultValues) {
  Armor armor;

  EXPECT_EQ(0, armor.id);
  EXPECT_EQ(0, armor.dex_id);
  EXPECT_EQ(PART_UNKNOWN, armor.part);
  EXPECT_THAT(armor.name,
              AllOf(Field(&LangText::eng, L""), Field(&LangText::chs, L""),
                    Field(&LangText::jpn, L"")));
  EXPECT_EQ(RANGE_UNKNOWN, armor.range);
  EXPECT_EQ(GENDER_UNKNOWN, armor.gender);
  EXPECT_EQ(0, armor.slots);
  EXPECT_EQ(1, armor.rare);
  EXPECT_EQ(0, armor.min_defense);
  EXPECT_EQ(0, armor.max_defense);
  EXPECT_THAT(armor.resist,
              AllOf(Field(&Resistance::fire, 0), Field(&Resistance::thunder, 0),
                    Field(&Resistance::dragon, 0), Field(&Resistance::water, 0),
                    Field(&Resistance::ice, 0)));
  EXPECT_THAT(armor.effect_list, ElementsAre());
}

TEST(JewelTest, DefaultValues) {
  Jewel jewel;

  EXPECT_EQ(0, jewel.id);
  EXPECT_EQ(0, jewel.dex_id);
  EXPECT_THAT(jewel.name,
              AllOf(Field(&LangText::eng, L""), Field(&LangText::chs, L""),
                    Field(&LangText::jpn, L"")));
  EXPECT_EQ(0, jewel.slots);
  EXPECT_THAT(jewel.effect_list, ElementsAre());
}

TEST(SkillTest, DefaultValues) {
  SkillTree skill_tree;

  EXPECT_EQ(0, skill_tree.id);
  EXPECT_EQ(0, skill_tree.dex_id);
  EXPECT_THAT(skill_tree.positives, ElementsAre());
  EXPECT_THAT(skill_tree.negatives, ElementsAre());
}

}  // namespace monster_avengers
