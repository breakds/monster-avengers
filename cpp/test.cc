#include "monster_hunter_data.h"
#include "query.h"
#include "armor_up.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:skill 50 10) (:skill 47 10) (:skill 103 15) (:weapon-type \"melee\")\n(:defense 240) (:weapon-holes 2) (:rare 8) (:amulet 1 (47 5))", 
                             &query));
  query.DebugPrint();
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4G");
  armor_up.Search(query, 10);
  armor_up.Summarize();
  return 0;
}
