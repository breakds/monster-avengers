#include <string>
#include "base/reindexed_table.h"

using namespace monster_avengers;
using namespace monster_avengers::dataset;

struct TestArmor {
  int rare;
  int slots;
  int defense;
};

void IndexMapTest() {
  IndexMap index_map;
  index_map.Update(2, 0);
  index_map.Update(3, 0);
  index_map.Update(4, 1);
  index_map.Update(6, 3);
  CHECK(0 == index_map.Internalize(3));
  CHECK(1 == index_map.Internalize(4));
  CHECK(3 == index_map.Internalize(6));
}

void ReindexedTableTest() {
  ReindexedTable<TestArmor> table;
  table.ResetIndexMap();

  table.Add(TestArmor({1, 1, 1}), 18);
  table.Add(TestArmor({2, 4, 8}), 20);
  table.Add(TestArmor({3, 6, 9}), 33);

  CHECK(1 == table[0].rare);
  CHECK(1 == table[0].slots);
  CHECK(1 == table[0].defense);
  CHECK(2 == table[1].rare);
  CHECK(4 == table[1].slots);
  CHECK(8 == table[1].defense);
  CHECK(3 == table[2].rare);
  CHECK(6 == table[2].slots);
  CHECK(9 == table[2].defense);

  ReindexedTable<std::string> secondary_table;
  secondary_table.ResetIndexMap(&table);

  secondary_table.Update("Great Name", 33);
  CHECK("Great Name" == secondary_table[2]);
}

int main() {
  IndexMapTest();
  ReindexedTableTest();
  Log(OK, L"Test reindexed_table_test completed.");
  return 0;
}
