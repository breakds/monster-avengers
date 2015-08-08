#include "dataset.h"
#include "supp/helpers.h"

using namespace monster_avengers;
using namespace monster_avengers::dataset;

int main() {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  Data::LoadSQLite("/home/breakds/Downloads/mh4g.db");
  for (int i = 0; i < Data::skills().size(); ++i) {
    Data::PrintSkill(i, 1);
  }
  for (int i = 0; i < Data::jewels().size(); ++i) {
    Data::PrintJewel(i, 1);
  }
  for (int i = 0; i < Data::armors().size(); ++i) {
    Data::PrintArmor(i, 1);
  }

  Log(OK, L"Test dataset_full_test completed.");
  return 0;
}
