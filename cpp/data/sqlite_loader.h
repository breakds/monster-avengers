#pragma once

#include <string>
#include "base/status.h"
#include "data/data.h"

namespace monster_avengers {

::base::Status LoadFromSqlite(const std::string &path, Data *data);

}  // namespace monster_avengers
