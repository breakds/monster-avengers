#include "utils/query.h"
#include "core/armor_up.h"
#include "supp/timer.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Data::LoadSQLite("/home/breakds/Downloads/mh4g.db");

  Timer timer;
  timer.Tic();
  Query query;
  double init_duration = timer.Toc();
  
  // CHECK_SUCCESS(Query::Parse(L"(:skill 51 10)"
  //                            L"(:skill 119 10)"
  //                            L"(:skill 47 15)"
  //                            L"(:skill 29 15)"
  //                            L"(:weapon-type \"melee\")"
  //                            L"(:weapon-slots 2)"
  //                            L"(:rare 8)"
  //                            L"(:amulet 0 (119 8))",
  //                            &query));
  

  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")"
  //                            L"(:weapon-slots 0)" 
  //                            L"(:rare 8)" 
  //                            L"(:max-rare 10)"
  //                            L"(:max-results 30)" 
  //                            L"(:skill 5 40)"
  //                            L"(:skill 30 20)",
  //                            &query));

  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
                             L"(:weapon-slots 2)"
                             L"(:skill 37 10)"
                             L"(:skill 42 10)"
                             L"(:skill 41 15)",
                             &query));
  
  // Query that returns 5 skills
  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
  //                            L"(:weapon-slots 2)"
  //                            L"(:rare 8)" 
  //                            L"(:skill 2 10)"
  //                            L"(:skill 39 15)"
  //                            L"(:skill 40 10)"
  //                            L"(:skill 138 10)" 
  //                            L"(:amulet 2 (2 4 21 5))",
  //                            &query));

  // Query that returns 6 skills
  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")" 
  //                            L"(:weapon-slots 2)"
  //                            L"(:skill 26 15)"
  //                            L"(:skill 2 10)"
  //                            L"(:skill 41 15)"
  //                            L"(:skill 42 10)"
  //                            L"(:skill 37 10)" 
  //                            L"(:skill 31 10)" 
  //                            L"(:amulet 2 (2 4 31 10))"
  //       		     L"(:gender \"female\")",
  //       		     // L"(:ban-jewels (163 164))",
  //                            &query));
  
  // Query that does not return anything (time consuming)
  // CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")"
  //                            L"(:weapon-slots 2)" 
  //                            L"(:rare 1)" 
  //                            L"(:skill 25 15)"
  //                            L"(:skill 38 20)"
  //                            L"(:skill 132 10)" 
  //                            L"(:skill 41 10)"
  //                            L"(:amulet 3 (38 5))"
  //                            L"(:amulet 2 (25 5))",
  //                            &query));

  // Debug Print for Query
  for (const Effect &effect : query.effects) {
    wprintf(L"require %d ", effect.points);
    Data::PrintSkill(effect.id, 0);
  }

  ArmorUp armor_up;
  timer.Tic();
  std::vector<ArmorSet> result = std::move(armor_up.Search(query));
  for (const ArmorSet &armor_set : result) {
    Data::PrintArmorSet(armor_set, armor_up.GetArsenal());
  }
  double duration = timer.Toc();
  armor_up.Summarize();
  wprintf(L"Initialization: %.4lf seconds.\n", init_duration);
  wprintf(L"Computation: %.4lf seconds.\n", duration);

  return 0;
}
