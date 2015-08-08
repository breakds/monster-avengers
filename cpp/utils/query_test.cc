#include <functional>

#include "dataset/dataset.h"
#include "query.h"

using namespace monster_avengers;

bool EffectEqual(const Effect &a, const Effect &b) {
  return std::tie(a.id, a.points) == std::tie(b.id, b.points);
}

template <typename ItemType>
bool SetEqual(std::function<bool(const ItemType&, const ItemType&)> compare,
              const std::vector<Effect> &a,
              const std::vector<Effect> &b) {
  // TODO(breakds): This logic is wrong, should be fixed.
  if (a.size() != b.size()) return false;
  for (int i = 0; i < a.size(); ++i) {
    bool found = false;
    for (int j = 0; j < b.size(); ++j) {
      if (compare(a[i], b[j])) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}

bool ArmorEqual(const Armor &a, const Armor &b) {
  return (std::tie(a.part, a.weapon_type, a.gender, a.rare,
                   a.min_defense, a.max_defense) ==
          std::tie(b.part, b.weapon_type, b.gender, b.rare,
                   b.min_defense, b.max_defense));
}


int main() {
  dataset::Data::LoadSQLite("/home/breakds/Downloads/mh4g.db");
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"range\")"
                             L"(:weapon-slots 2)"
                             L"(:skill 25 15)"
                             L"(:skill 1 10)"
                             L"(:skill 40 15)"
                             L"(:skill 41 10)"
                             L"(:skill 36 10)" 
                             L"(:skill 30 10)"
                             L"(:max-results 20)"
                             L"(:amulet 2 (1 4 30 10))"
                             L"(:gender \"female\")",
                             &query));
  CHECK(SetEqual<Effect>(EffectEqual, {
        {Data::skills().Internalize(25),15},
        {Data::skills().Internalize(1),10},
        {Data::skills().Internalize(40),15},
        {Data::skills().Internalize(41),10},
        {Data::skills().Internalize(36),10},
        {Data::skills().Internalize(30),10}},
      query.effects));
  CHECK(query.max_results == 20);
  CHECK(query.defense == 0);
  CHECK(query.armor_filter.weapon_type == WEAPON_TYPE_RANGE);
  CHECK(query.armor_filter.weapon_slots == 2);
  CHECK(query.armor_filter.gender == GENDER_FEMALE);

  Armor amulet1 = Data::CreateAmulet({{1, 4}, {30, 10}}, 2);
  Armor amulet2 = Data::CreateAmulet({{1, 4}, {30, 10}}, 2);
  ArmorEqual(amulet1, amulet2);
  
  Log(OK, L"Test query_test completed.");
  return 0;
}
