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
std::array<std::vector<int>, PART_NUM> Data::armor_by_parts_;

void Data::AddPredefinedArmors() {
  int external_id = 170000;

  // Gears
  for (int i = 0; i < 4; ++i) {
    Armor armor;
    ArmorAddon addon;
    armor.part = GEAR;
    armor.gender = GENDER_BOTH;
    armor.rare = 10;
    armor.min_defense = 0;
    armor.max_defense = 0;
    armor.resistance = Resistance{0, 0, 0, 0, 0};
    armor.slots = i;
    addon.name[ENGLISH] = L"Weapon";
    addon.name[CHINESE] = L"Weapon";
    addon.name[JAPANESE] = L"Weapon";
    armors_.Add(armor, ++external_id);
    armor_addons_.Update(addon, external_id);
  }

  // Amulets
  {
    Armor amulet;
    ArmorAddon addon;
    amulet.part = AMULET;
    amulet.gender = GENDER_BOTH;
    amulet.rare = 10;
    amulet.min_defense = 0;
    amulet.max_defense = 0;
    amulet.resistance = Resistance{0, 0, 0, 0, 0};
    amulet.slots = 0;
    addon.name[ENGLISH] = L"Amulet";
    addon.name[CHINESE] = L"Amulet";
    addon.name[JAPANESE] = L"Amulet";
    armors_.Add(amulet, ++external_id);
    armor_addons_.Update(addon, external_id);
  }
}

void Data::ClassifyParts() {
  for (int i = 0; i < PART_NUM; ++i) {
    armor_by_parts_[i].clear();
  }
  for (int i = 0; i < armors_.size(); ++i) {
    armor_by_parts_[armors_[i].part].push_back(i);
  }
}

}  // namespace dataset

}  // namespace monster_avengers
