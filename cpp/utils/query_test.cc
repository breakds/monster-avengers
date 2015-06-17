#include "query.h"

using namespace monster_avengers;

int main() {
  dataset::Data::LoadSQLite("/home/breakds/Downloads/mh4g.db");
  Query query;
  CHECK_SUCCESS(Query::Parse(L"(:weapon-type \"melee\")"
                             L"(:weapon-slots 2)"
                             L"(:skill 25 15)"
                             L"(:skill 1 10)"
                             L"(:skill 40 15)"
                             L"(:skill 41 10)"
                             L"(:skill 36 10)" 
                             L"(:skill 30 10)"
                             L"(:amulet 2 (1 4 30 10))"
                             L"(:gender \"female\")"
                             L"(:ban-jewels (29 200))",
                             &query));
  Log(OK, L"Test query_test completed.");
  return 0;
}
