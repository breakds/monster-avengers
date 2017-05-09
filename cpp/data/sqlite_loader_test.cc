#include "data/sqlite_loader.h"

#include "base/status_test_util.h"
#include "data/types/properties.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;
using ::testing::AllOf;
using ::testing::Field;

namespace monster_avengers {
namespace data {

MATCHER_P3(LangTextEq, eng, chs, jpn, "") {
  if (eng != arg.eng) {
    (*result_listener) << "eng does not match. ";
    return false;
  }
  if (chs != arg.chs) {
    (*result_listener) << "chs does not match.";
    return false;
  }
  if (jpn != arg.jpn) {
    (*result_listener) << "jpn does not match.";
    return false;
  }
  return true;
}

MATCHER_P2(SkillEq, eng, points, "") {
  if (eng != arg.name.eng) {
    (*result_listener) << "Skill name does not match (eng).";
    return false;
  }
  if (points != arg.points) {
    (*result_listener) << "Skill points does not match.";
    return false;
  }
  return true;
}

TEST(SQLiteLoader, main) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Data data;
  EXPECT_OK(LoadFromSqlite("data/testdata/test.db", &data));

  // Only test specific skills for the first and last skill trees.
  EXPECT_THAT(
      data.skill_trees,
      ElementsAre(
          AllOf(Field(&SkillTree::id, 0), Field(&SkillTree::dex_id, 2),
                Field(&SkillTree::name, LangTextEq(L"Poison", L"毒", L"毒")),
                Field(&SkillTree::positives,
                      UnorderedElementsAre(SkillEq(L"Negate Poison", 10))),
                Field(&SkillTree::negatives,
                      UnorderedElementsAre(SkillEq(L"Double Poison", -10)))),
          AllOf(Field(&SkillTree::id, 1), Field(&SkillTree::dex_id, 3),
                Field(&SkillTree::name,
                      LangTextEq(L"Paralysis", L"麻痹", L"麻痺"))),
          AllOf(
              Field(&SkillTree::id, 2), Field(&SkillTree::dex_id, 4),
              Field(&SkillTree::name, LangTextEq(L"Sleep", L"睡眠", L"睡眠"))),
          AllOf(Field(&SkillTree::id, 3), Field(&SkillTree::dex_id, 5),
                Field(&SkillTree::name, LangTextEq(L"Stun", L"晕眩", L"気絶"))),
          AllOf(Field(&SkillTree::id, 4), Field(&SkillTree::dex_id, 9),
                Field(&SkillTree::name,
                      LangTextEq(L"Bind Res", L"雪人", L"だるま"))),
          AllOf(
              Field(&SkillTree::id, 5), Field(&SkillTree::dex_id, 21),
              Field(&SkillTree::name, LangTextEq(L"Health", L"体力", L"体力"))),
          AllOf(Field(&SkillTree::id, 6), Field(&SkillTree::dex_id, 128),
                Field(&SkillTree::name,
                      LangTextEq(L"Botany", L"野草知识", L"野草知識"))),
          AllOf(Field(&SkillTree::id, 7), Field(&SkillTree::dex_id, 132),
                Field(&SkillTree::name,
                      LangTextEq(L"Gathering", L"采集", L"採取"))),
          AllOf(Field(&SkillTree::id, 8), Field(&SkillTree::dex_id, 135),
                Field(&SkillTree::name,
                      LangTextEq(L"Whim", L"反复无常", L"気まぐれ")),
                Field(&SkillTree::positives,
                      UnorderedElementsAre(SkillEq(L"Divine Whim", 15),
                                           SkillEq(L"Spirit's Whim", 10))),
                Field(&SkillTree::negatives,
                      UnorderedElementsAre(SkillEq(L"Spectre's Whim", -10),
                                           SkillEq(L"Devil's Whim", -15))))));

  EXPECT_THAT(
      std::vector<Jewel>(data.jewels.begin(), data.jewels.begin() + 2),
      ElementsAre(
          AllOf(
              Field(&Jewel::id, 0), Field(&Jewel::dex_id, 1),
              Field(&Jewel::slots, 1),
              Field(&Jewel::name, LangTextEq(L"Antidote Jwl 1", L"耐毒珠【１】",
                                             L"耐毒珠【１】")),
              Field(&Jewel::effect_list,
                    UnorderedElementsAre(
                        AllOf(Field(&Effect::id, 0), Field(&Effect::points, 1)),
                        AllOf(Field(&Effect::id, 3),
                              Field(&Effect::points, -1))))),
          AllOf(
              Field(&Jewel::id, 1), Field(&Jewel::dex_id, 3),
              Field(&Jewel::slots, 2),
              Field(&Jewel::name, LangTextEq(L"Antidote Jwl 2", L"耐毒珠【２】",
                                             L"耐毒珠【２】")),
              Field(&Jewel::effect_list,
                    UnorderedElementsAre(
                        AllOf(Field(&Effect::id, 0), Field(&Effect::points, 3)),
                        AllOf(Field(&Effect::id, 3),
                              Field(&Effect::points, -1)))))));

  EXPECT_THAT(
      data.armors,
      ElementsAre(
          AllOf(Field(&Armor::id, 0), Field(&Armor::dex_id, 1),
                Field(&Armor::part, HEAD),
                Field(&Armor::name, LangTextEq(L"Leather Headgear", L"轻皮头盔",
                                               L"レザーヘッド")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 1),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 1),
                Field(&Armor::max_defense, 25),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 0),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 8),
                                                 Field(&Effect::points, 2))))),
          AllOf(Field(&Armor::id, 1), Field(&Armor::dex_id, 2),
                Field(&Armor::part, BODY),
                Field(&Armor::name, LangTextEq(L"Leather Vest", L"轻皮背心",
                                               L"レザーベスト")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 0),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 1),
                Field(&Armor::max_defense, 25),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 0),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 8),
                                                 Field(&Effect::points, 5))))),
          AllOf(Field(&Armor::id, 2), Field(&Armor::dex_id, 3),
                Field(&Armor::part, HANDS),
                Field(&Armor::name, LangTextEq(L"Leather Gloves", L"轻皮手套",
                                               L"レザーグラブ")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 1),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 1),
                Field(&Armor::max_defense, 25),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 0),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 8),
                                                 Field(&Effect::points, 3))))),
          AllOf(Field(&Armor::id, 3), Field(&Armor::dex_id, 4),
                Field(&Armor::part, WAIST),
                Field(&Armor::name, LangTextEq(L"Leather Belt", L"轻皮扣带",
                                               L"レザーベルト")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 2),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 1),
                Field(&Armor::max_defense, 25),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 0),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 7),
                                                 Field(&Effect::points, 2))))),
          AllOf(Field(&Armor::id, 4), Field(&Armor::dex_id, 5),
                Field(&Armor::part, FEET),
                Field(&Armor::name, LangTextEq(L"Leather Trousers", L"轻皮具足",
                                               L"レザーパンツ")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 1),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 1),
                Field(&Armor::max_defense, 25),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 0),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 7),
                                                 Field(&Effect::points, 3))))),
          AllOf(Field(&Armor::id, 5), Field(&Armor::dex_id, 6),
                Field(&Armor::part, HEAD),
                Field(&Armor::name, LangTextEq(L"Chainmail Headgear",
                                               L"锁链头盔", L"チェーンヘッド")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 3),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 2),
                Field(&Armor::max_defense, 26),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 1),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list, UnorderedElementsAre())),
          AllOf(Field(&Armor::id, 6), Field(&Armor::dex_id, 7),
                Field(&Armor::part, BODY),
                Field(&Armor::name, LangTextEq(L"Chainmail Vest", L"锁链背心",
                                               L"チェーンベスト")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 1),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 2),
                Field(&Armor::max_defense, 26),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 1),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 5),
                                                 Field(&Effect::points, 3))))),
          AllOf(Field(&Armor::id, 7), Field(&Armor::dex_id, 8),
                Field(&Armor::part, HANDS),
                Field(&Armor::name, LangTextEq(L"Chainmail Gloves", L"锁链手套",
                                               L"チェーングラブ")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 1),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 2),
                Field(&Armor::max_defense, 26),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 1),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 5),
                                                 Field(&Effect::points, 2))))),
          AllOf(Field(&Armor::id, 8), Field(&Armor::dex_id, 9),
                Field(&Armor::part, WAIST),
                Field(&Armor::name, LangTextEq(L"Chainmail Belt", L"锁链扣带",
                                               L"チェーンベルト")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 1),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 2),
                Field(&Armor::max_defense, 26),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 1),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list,
                      UnorderedElementsAre(AllOf(Field(&Effect::id, 6),
                                                 Field(&Effect::points, 2))))),
          AllOf(Field(&Armor::id, 9), Field(&Armor::dex_id, 10),
                Field(&Armor::part, FEET),
                Field(&Armor::name, LangTextEq(L"Chainmail Trousers",
                                               L"锁链具足", L"チェーンパンツ")),
                Field(&Armor::range, RANGE_BOTH),
                Field(&Armor::gender, GENDER_BOTH), Field(&Armor::slots, 0),
                Field(&Armor::rare, 1), Field(&Armor::min_defense, 2),
                Field(&Armor::max_defense, 26),
                Field(&Armor::resist, AllOf(Field(&Resistance::fire, 0),
                                            Field(&Resistance::thunder, 1),
                                            Field(&Resistance::dragon, 0),
                                            Field(&Resistance::water, 0),
                                            Field(&Resistance::ice, 0))),
                Field(&Armor::effect_list, UnorderedElementsAre(AllOf(
                                               Field(&Effect::id, 5),
                                               Field(&Effect::points, 5)))))));
}

}  // namespace data
}  // namespace monster_avengers
