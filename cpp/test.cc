#include "monster_hunter_data.h"
#include "query.h"
#include "armor_up.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:skill 5 10) (:skill 12 10) (:weapon-type \"melee\")\n(:defense 130)", 
                             &query));
  query.DebugPrint();
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4");
  armor_up.SearchFoundation(query);
  armor_up.Summarize();
  return 0;
}
