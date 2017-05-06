#include "data/sqlite_loader.h"

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
  LoadFromSqlite("data/testdata/test.db", &data);

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
}

}  // namespace data
}  // namespace monster_avengers
