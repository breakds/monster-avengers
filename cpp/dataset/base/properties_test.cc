#include <string>

#include "base/data_utils.h"
#include "base/properties.h"
#include "supp/helpers.h"

using namespace monster_avengers::dataset;
using namespace monster_avengers;

void EnumTest() {
  CHECK("hands" == StringifyEnum<ArmorPart>(HANDS));
  CHECK(WEAPON_TYPE_BOTH == ParseEnum<WeaponType>("both"));
}


int main() {
  EnumTest();
  Log(OK, L"Test properties_test completed.");
  return 0;
}
