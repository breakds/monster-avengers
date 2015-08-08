#include <string>
#include <vector>

#include "core/armor_up.h"
#include "dataset/dataset.h"
#include "supp/helpers.h"
#include "supp/timer.h"
#include "utils/formatter.h"
#include "verifier.h"

using namespace monster_avengers;
using namespace monster_avengers::testing;

using dataset::Data;

// ---------- Utilities ----------

std::string GetTestName(char *filename) {
  int slash_pos = 0;
  int dot_pos = 0;
  int i = 0;
  while (filename[i] != 0) {
    if ('/' == filename[i]) {
      slash_pos = i;
    }
    if ('.' == filename[i]) {
      dot_pos = i;
    }
    ++i;
  }

  // Initialize test_name with spaces.
  std::string test_name(dot_pos - slash_pos - 1, ' ');

  i = slash_pos;
  for (char &character : test_name) character = filename[++i];

  return test_name;
}

inline bool IsPrefix(const std::string prefix,
                     const std::string target) {
  auto prefix_iter = prefix.begin();
  auto target_iter = target.begin();
  while ((prefix_iter != prefix.end()) &&
         (target_iter != target.end()) &&
         (*(prefix_iter++) == *(target_iter++)));
  return prefix.end() == prefix_iter;
}

std::string LoadDataSet(const std::string &spec) {
  static std::string sqlite_prefix = "sqlite:";
  static std::string binary_prefix = "binary:";
  
  // Try matching sqlite prefix.
  if (IsPrefix(sqlite_prefix, spec)) {
    Data::LoadSQLite(spec.substr(sqlite_prefix.size()));
    return "sqlite";
  }

  // Try matching binary prefix.
  if (IsPrefix(binary_prefix, spec)) {
    Data::LoadBinary(spec.substr(binary_prefix.size()));
    return "binary";
  }

  // Default, sqlite loader
  Data::LoadSQLite(spec);
  return "sqlite";
}

// Arguments:
// 1 - data set location
// 2+ - regression query names
int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  
  if (argc < 2) {
    Log(FATAL, L"Invalid number of arguments.");
  }

  // Load Data set
  std::string data_type = LoadDataSet(argv[1]);

  // Prepare Timer
  Timer timer;

  // Build Search Engine
  ArmorUp armor_up;

  Query query;

  for (int i = 2; i < argc; ++i) {
    CHECK_SUCCESS(Query::ParseFile(argv[i], &query));

    std::string test_name = GetTestName(argv[i]);
    wprintf(L"+--------------------------------------+\n");
    wprintf(L"|          Test: %s", test_name.c_str());
    std::string padding(22 - test_name.size(), ' ');
    wprintf(L"%s|\n", padding.c_str());
    wprintf(L"+--------------------------------------+\n");

    timer.Tic();
    std::vector<ArmorSet> result = std::move(armor_up.Search(query));
    double duration = timer.Toc();
    Log(INFO, L"Time elapsed for query: %.4lf seconds.", duration);

    // Just output in Dex format if we accept binary data.
    if ("binary" == data_type) {
      DexFormatter formatter(&armor_up.GetArsenal());
      wprintf(L"%s\n", formatter.StringBatchFormat(result).c_str());
    }
    
    int num_passed = 0;
    for (const ArmorSet &armor_set : result) {
      std::vector<VerificationMessage> errors =
          VerifyArmorSet(armor_up.GetArsenal(), query, armor_set);
      if (errors.empty()) {
        num_passed++;
      }

      if (argc == 3 || !errors.empty()) {
        if ("sqlite" == data_type) {
          wprintf(L"\n");
          Data::PrintArmorSet(armor_set, armor_up.GetArsenal(), 1, CHINESE);
        }
      } else {
        continue;
      }

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
