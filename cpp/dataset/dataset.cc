#include "dataset.h"

namespace monster_avengers {

namespace dataset {

ReindexedTable<Armor> Data::armors_;
ReindexedTable<Jewel> Data::jewels_;
ReindexedTable<SkillSystem> Data::skills_;
ReindexedTable<Item> Data::items_;
ReindexedTable<ArmorAddon> Data::armor_addons_;
ReindexedTable<JewelAddon> Data::jewel_addons_;
ReindexedTable<SkillSystemAddon> Data::skill_addons_;

}  // namespace dataset

}  // namespace monster_avengers
