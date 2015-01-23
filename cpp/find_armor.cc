#include "monster_hunter_data.h"
#include "query.h"
#include "armor_up.h"
#include "helpers.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");

  if (argc < 2) {
    Log(ERROR, L"Please specify the query file.");
  }
  Query query;
  CHECK_SUCCESS(Query::ParseFile(argv[1], &query));
  ArmorUp armor_up("/home/breakds/pf/projects/monster-avengers/dataset/MH4");
  armor_up.SearchAndOutput(query, 10);
  return 0;
}
