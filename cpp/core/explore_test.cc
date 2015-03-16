#include "data/data_set.h"
#include "utils/query.h"
#include "core/armor_up.h"
#include "aux/timer.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
                             L"(:weapon-holes 2)"
                             L"(:skill 36 10)"
                             L"(:skill 41 10)"
                             L"(:skill 40 15)"
                             L"(:skill 30 10)", 
                             &query));

  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"range\")"
  //                            L"(:weapon-holes 0)"
  //                            L"(:rare 1)"
  //                            L"(:max-rare 10)"
  //                            L"(:max-results 10)"
  //                            L"(:skill 9 10)"
  //                            L"(:skill 52 10)"
  //                            L"(:skill 56 10)"
  //                            L"(:skill 57 10)"
  //                            // L"(:skill 132 10)"
  //                            L"(:amulet 1 (52 6 57 4))",
  //                            &query));

  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")"
  //                            L"(:weapon-holes 2)" 
  //                            L"(:rare 1)" 
  //                            L"(:skill 25 15)"
  //                            L"(:skill 38 20)"
  //                            // L"(:skill 132 10)" 
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
  armor_up.Explore(query);
  double duration = timer.Toc();
  armor_up.Summarize();
  wprintf(L"Initialization: %.4lf seconds.\n", init_duration);
  wprintf(L"Computation: %.4lf seconds.\n", duration);

  return 0;
}
