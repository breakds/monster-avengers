#include "data/sqlite_loader.h"

#include "gtest/gtest.h"

namespace monster_avengers {
namespace data {

TEST(SQLiteLoader, main) {
  LoadFromSqlite("data/testdata/test.db");
}

}  // namespace data
}  // namespace monster_avengers
