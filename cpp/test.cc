#include "monster_hunter_data.h"
#include "query.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  DataSet dataset("/home/breakds/pf/projects/monster-avengers/dataset/MH4");
  dataset.Summarize();
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:skill 3 2)(:skill 5 5) (:weapon-type \"melee\")\n(:defense 130)", 
                             &query));
  query.DebugPrint();
  return 0;
}
