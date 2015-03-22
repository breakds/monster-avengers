#include "data/data_set.h"
#include "utils/query.h"
#include "core/armor_up.h"
#include "supp/timer.h"

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
  

  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")"
  //                            L"(:weapon-holes 0)" 
  //                            L"(:rare 8)" 
  //                            L"(:max-rare 10)"
  //                            L"(:max-results 30)" 
  //                            L"(:skill 5 40)"
  //                            L"(:skill 30 20)",
  //                            &query));

  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
  //                            L"(:weapon-holes 2)"
  //                            L"(:skill 36 10)"
  //                            L"(:skill 41 10)"
  //                            L"(:skill 40 15)",
  //                            &query));
  
  // Query that returns 5 skills
  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
  //                            L"(:weapon-holes 2)"
  //                            L"(:rare 8)" 
  //                            L"(:skill 1 10)"
  //                            L"(:skill 40 15)"
  //                            L"(:skill 41 10)"
  //                            L"(:skill 137 10)" 
  //                            L"(:amulet 2 (1 4 20 5))",
  //                            &query));
  
  // Query that returns 6 skills
  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
                             L"(:weapon-holes 2)"
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
  //                            L"(:skill 25 15)"
  //                            L"(:skill 38 20)"
  //                            L"(:skill 132 10)" 
  //                            L"(:skill 41 10)"
  //                            L"(:amulet 3 (38 5))"
  //                            L"(:amulet 2 (25 5))",
  //                            &query));
  
  query.DebugPrint();
  Timer timer;
  timer.Tic();
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4GU");
  double init_duration = timer.Toc();
  timer.Tic();
  armor_up.Search<SCREEN>(query);
  double duration = timer.Toc();
  armor_up.Summarize();
  wprintf(L"Initialization: %.4lf seconds.\n", init_duration);
  wprintf(L"Computation: %.4lf seconds.\n", duration);

  return 0;
}
