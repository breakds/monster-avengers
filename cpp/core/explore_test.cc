#include "data/data_set.h"
#include "utils/query.h"
#include "core/armor_up.h"
#include "aux/timer.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Query query;
  // Query that returns 6 skills
  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
                             L"(:weapon-holes 2)"
                             L"(:rare 8)" 
                             L"(:skill 25 15)"
                             L"(:skill 1 10)"
                             // L"(:skill 40 15)"
                             // L"(:skill 36 10)" 
                             // L"(:skill 30 10)" 
                             L"(:amulet 2 (1 4 30 10))",
                             &query));
  
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
