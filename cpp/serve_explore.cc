#include "data/data_set.h"
#include "utils/query.h"
#include "core/armor_up.h"
#include "supp/helpers.h"
#include "supp/timer.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  CHECK(2 <= argc);
  ArmorUp armor_up(argv[1]);
  if (argc < 4) {
    armor_up.ListSkills();
  } else {
    Query query;
    CHECK_SUCCESS(Query::ParseFile(argv[2], &query));
    Timer timer;
    timer.Tic();
    armor_up.Explore(query, argv[3]);
    double duration = timer.Toc();
    wprintf(L"Computation: %.4lf seconds.\n", duration);
  }
  return 0;
}
