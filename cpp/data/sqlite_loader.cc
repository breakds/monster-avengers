#include "data/sqlite_loader.h"

#include <sqlite3.h>
#include <unordered_map>
#include "data/data.h"
#include "glog/logging.h"

namespace monster_avengers {
namespace data {

namespace {
typedef int (*Callback)(void *, int, char **, char **);
// using Callback = std::function<int(void *, int, char **, char **)>;

void RunSQLiteQuery(sqlite3 *database, Data *data, const char *query,
                    Callback callback) {
  char *error = nullptr;

  int status = sqlite3_exec(database, query, callback,
                            static_cast<void *>(data), &error);

  if (SQLITE_OK != status) {
    LOG(ERROR) << "Sqlite Query Error" << error;
    sqlite3_free(error);
    LOG(FATAL) << "Terminate because the previous error.";
  }
}

template <typename ValueType>
ValueType ConvertTo(const std::string &original) {
  static_assert("Not implemented.");
}

template <>
int ConvertTo<int>(const std::string &original) {
  return std::stoi(original);
}

// template <>
// std::string ConvertTo<std::string>(const std::string &original) {
//   return original;
// }

template <>
std::wstring ConvertTo<std::wstring>(const std::string &original) {
#if _WIN32
  wchar_t buffer[255];
#else
  wchar_t buffer[original.size()];
#endif
  swprintf(buffer, original.size(), L"%s", original.c_str());
  return buffer;
}

class Row {
 public:
  Row(int argc, char **argv, char **caption) : result_() {
    for (int i = 0; i < argc; ++i) {
      if (nullptr != argv[i]) {
        result_[caption[i]] = argv[i];
      } else {
        result_[caption[i]] = "";
      }
    }
  }

  template <typename ValueType>
  ValueType Get(const std::string &key) {
    return ConvertTo<ValueType>(result_[key]);
  }

 private:
  std::unordered_map<std::string, std::string> result_;
};
}  // namespace

void LoadFromSqlite(const std::string &path) {
  sqlite3 *database;
  CHECK(!sqlite3_open(path.c_str(), &database))
      << "Failed to open the dex sqlite file.";

  // Initialize the empty data.
  Data data;

  RunSQLiteQuery(database, &data,
                 "SELECT SklTree_ID AS dex_id, "
                 "       SklTree_Name_0 AS eng, "
                 "       SklTree_Name_1 AS chs, "
                 "       SklTree_Name_3 AS jpn "
                 "FROM ID_SklTree_Name;",
                 [](void *data, int argc, char **argv, char **caption) {
                   Data &result = *static_cast<Data *>(data);

                   Row row(argc, argv, caption);
                   result.skill_trees.emplace_back();
                   SkillTree &skill_tree = result.skill_trees.back();
                   skill_tree.id =
                       static_cast<int>(result.skill_trees.size() - 1);
                   skill_tree.dex_id = row.Get<int>("dex_id");
                   skill_tree.name.eng = row.Get<std::wstring>("eng");
                   skill_tree.name.chs = row.Get<std::wstring>("chs");
                   skill_tree.name.jpn = row.Get<std::wstring>("jpn");
                   return 0;
                 });
  sqlite3_close(database);
}

}  // namespace data
}  // namespace monster_avengers
