#include "monster_hunter_data.h"
#include "query.h"
#include "armor_up.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Query query;
  // CHECK_SUCCESS(Query::Parse(L"(:skill 51 10)"
  //                            L"(:skill 119 10)"
  //                            L"(:skill 47 15)"
  //                            L"(:skill 29 15)"
  //                            L"(:weapon-type \"melee\")"
  //                            L"(:weapon-holes 2)"
  //                            L"(:rare 8)"
  //                            L"(:amulet 0 (119 8))",
  //                            &query));
  // CHECK_SUCCESS(Query::Parse(L"(:skill 133 20)"
  //                            L"(:skill 44 10)"
  //                            L"(:skill 51 10)"
  //                            L"(:skill 119 10)"
  //                            L"(:weapon-type \"melee\")"
  //                            L"(:defense 440)"
  //                            L"(:rare 8)"
  //                            L"(:amulet 0 (133 6))"
  //                            L"(:amulet 3 (133 5))"
  //                            L"(:amulet 2 (40 5))"
  //                            L"(:amulet 2 (114 8))"
  //                            L"(:amulet 1 (119 8))"
  //                            L"(:blacklist (1575 502))",
  //                            &query));
  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
                             L"(:weapon-holes 2)"
                             L"(:rare 1)" 
                             L"(:skill 47 15)"
                             L"(:skill 39 10)"
                             L"(:skill 40 15)"
                             L"(:skill 113 10)"
                             L"(:skill 50 10)" 
                             L"(:amulet 2 (39 4 20 5))",
                             &query));
  query.DebugPrint();
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4G");
  armor_up.Search(query, 10);
  armor_up.Summarize();
  return 0;
}
