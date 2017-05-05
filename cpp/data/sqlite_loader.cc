#include "data/sqlite_loader.h"

#include <sqlite3.h>
#include "glog/logging.h"

namespace monster_avengers {
namespace data {

typedef int (*Callback)(void *, int, char **, char **);
// using Callback = std::function<int(void *, int, char **, char **)>;

void RunSQLiteQuery(sqlite3 *database, const char *query, Callback callback) {
  char *error = nullptr;

  int status = sqlite3_exec(database, query, callback, nullptr, &error);

  if (SQLITE_OK != status) {
    LOG(ERROR) << "Sqlite Query Error" << error;
    sqlite3_free(error);
    LOG(FATAL) << "Terminate because the previous error.";
  }
}

void LoadFromSqlite(const std::string &path) {
  sqlite3 *database;
  CHECK(!sqlite3_open(path.c_str(), &database))
      << "Failed to open the dex sqlite file.";

  // std::vector<SkillTree> skill_trees;

  RunSQLiteQuery(database,
                 "SELECT SklTree_ID AS id "
                 "FROM ID_SklTree_Name;",
                 [](void *data, int argc, char **argv, char **caption) {
                   for (int i = 0; i < argc; ++i) {
                     LOG(INFO) << caption[i];
                   }
                   return 0;
                 });
  sqlite3_close(database);
}

}  // namespace data
}  // namespace monster_avengers
