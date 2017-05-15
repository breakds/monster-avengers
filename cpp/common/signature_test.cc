#include "common/signature.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAre;

namespace monster_avengers {

TEST(SignatureTest, ConstructFromEffectListSimple) {
  EffectList effect_list{Effect(12, -10), Effect(7, 30)};
  SkillTreeIdList ids{2, 7, 17, 3};
  Signature signature(effect_list, ids);
  EXPECT_THAT(signature.values, ElementsAre(0, 30, 0, 0));
}

TEST(SignatureTest, ConstructFromEffectListNegative) {
  EffectList effect_list{Effect(3, -10), Effect(7, 30)};
  SkillTreeIdList ids{2, 7, 17, 3};
  Signature signature(effect_list, ids);
  EXPECT_THAT(signature.values, ElementsAre(0, 30, 0, -10));
}

TEST(SignatureTest, HasPositive) {
  EffectList effect_list{Effect(3, -10), Effect(7, 30)};
  SkillTreeIdList ids{2, 7, 17, 3};
  Signature signature(effect_list, ids);
  EXPECT_TRUE(signature.HasPositive());
}

TEST(SignatureTest, HasNoPositive) {
  EffectList effect_list{Effect(3, -10), Effect(7, -30)};
  SkillTreeIdList ids{2, 7, 17, 3};
  Signature signature(effect_list, ids);
  EXPECT_FALSE(signature.HasPositive());
}

}  // namespace monster_avengers
