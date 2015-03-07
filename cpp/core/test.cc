#include "data/data_set.h"
#include "utils/query.h"
#include "core/armor_up.h"
#include "aux/timer.h"

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

  // Query that returns 5 skills
  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
  //                            L"(:weapon-holes 2)"
  //                            L"(:rare 8)" 
  //                            L"(:skill 25 15)"
  //                            L"(:skill 1 10)"
  //                            L"(:skill 40 15)"
  //                            L"(:skill 41 10)"
  //                            L"(:skill 36 10)" 
  //                            L"(:amulet 2 (1 4 20 5))",
  //                            &query));
  
  // Query that returns 6 skills
  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
                             L"(:weapon-holes 2)"
                             L"(:rare 8)" 
                             L"(:skill 25 15)"
                             L"(:skill 1 10)"
                             L"(:skill 40 15)"
                             L"(:skill 41 10)"
                             L"(:skill 36 10)" 
                             L"(:skill 30 10)" 
                             L"(:amulet 2 (1 4 30 10))",
                             &query));

  // Query that does not return anything (time consuming)
  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")"
  //                            L"(:weapon-holes 2)" 
  //                            L"(:rare 1)" 
  //                            L"(:skill 41 10)"
  //                            L"(:skill 38 20)"
  //                            L"(:skill 25 15)"
  //                            L"(:skill 132 10)" 
  //                            L"(:amulet 3 (38 5))"
  //                            L"(:amulet 2 (25 5))",
  //                            &query));
  
  query.DebugPrint();
  Timer timer;
  timer.Tic();
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4GU");
  double init_duration = timer.Toc();
  timer.Tic();
  armor_up.Search(query, 10);
  double duration = timer.Toc();
  armor_up.Summarize();
  wprintf(L"Initialization: %.4lf seconds.\n", init_duration);
  wprintf(L"Computation: %.4lf seconds.\n", duration);

  return 0;
}
