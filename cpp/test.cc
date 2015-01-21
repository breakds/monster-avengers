#include "monster_hunter_data.h"
#include "query.h"
#include "armor_up.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:skill 46 10) (:skill 43 10) (:skill 91 15) (:weapon-type \"melee\")\n(:defense 240)", 
                             &query));
  query.DebugPrint();
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4");
  armor_up.Search(query, 10);
  armor_up.Summarize();
  return 0;
}
