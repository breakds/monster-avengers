#include "monster_hunter_data.h"
#include "query.h"
#include "armor_up.h"
#include "helpers.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  ArmorUp armor_up("MH4G/");
  
  if (argc < 2) {
    armor_up.ListSkills();
  } else {
    Query query;
    CHECK_SUCCESS(Query::ParseFile(argv[1], &query));
    armor_up.SearchAndOutput(query, 10);
  }
  return 0;
}
