#include "monster_hunter_data.h"
#include "query.h"
#include "armor_up.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:skill 51 10)"
                             L"(:skill 119 10)"
                             L"(:skill 47 15)"
                             L"(:skill 29 15)"
                             L"(:weapon-type \"melee\")"
                             L"(:weapon-holes 2)"
                             L"(:rare 8)"
                             L"(:amulet 0 (119 8))",
                             &query));
  query.DebugPrint();
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4G");
  armor_up.Search(query, 10);
  armor_up.Summarize();
  return 0;
}
