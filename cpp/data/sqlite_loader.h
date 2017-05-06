#pragma once

#include <string>
#include "base/status.h"
#include "data/data.h"

namespace monster_avengers {
namespace data {

::base::Status LoadFromSqlite(const std::string &path, Data *data);

}  // namespace data
}  // namespace monster_avengers
