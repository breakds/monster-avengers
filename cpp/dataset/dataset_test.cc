#include "dataset.h"
#include "supp/helpers.h"

using namespace monster_avengers;
using namespace monster_avengers::dataset;

int main() {
  Data::LoadSQLite("/home/breakds/Downloads/mh4g.db");
  Log(OK, L"Test dataset_full_test completed.");
  return 0;
}
