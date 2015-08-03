#include <vector>

#include "core/armor_up.h"
#include "dataset/dataset.h"
#include "supp/helpers.h"
#include "supp/timer.h"
#include "verifier.h"

using namespace monster_avengers;
using namespace monster_avengers::testing;

using dataset::Data;

// Arguments:
// 1 - data set location
// 2+ - regression query names
int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  
  if (argc < 2) {
    Log(FATAL, L"Invalid number of arguments.");
  }

  // Load Data set
  Data::LoadSQLite(argv[1]);

  // Build Search Engine
  ArmorUp armor_up;

  Query query;

  for (int i = 2; i < argc; ++i) {
    CHECK_SUCCESS(Query::ParseFile(argv[i], &query));
    
    std::vector<ArmorSet> result = std::move(armor_up.Search(query));

    int num_passed = 0;
    for (const ArmorSet &armor_set : result) {
      std::vector<VerificationMessage> errors =
          VerifyArmorSet(armor_up.GetArsenal(), query, armor_set);
      if (errors.empty()) {
        num_passed++;
        continue;
      }

      wprintf(L"\n------------------------------------------------------------\n");
      Data::PrintArmorSet(armor_set, armor_up.GetArsenal(), 1, CHINESE);

      for (const VerificationMessage &message : errors) {
        message.Print();
      }
    }
    if (result.size() == num_passed) {
      Log(OK, L"%d/%llu Passed, for %s.", num_passed,
          result.size(), argv[i]);
    } else {
      Log(FATAL, L"%d/%llu Passed, for %s.", num_passed,
          result.size(), argv[i]);
    }
  }

  return 0;
}
