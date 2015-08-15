#include <cmath>

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

void Data::AddPredefinedArmors(bool with_addon) {
  int external_id = 170000;

  // Gears
  for (int i = 0; i < 4; ++i) {
    Armor armor;
    armor.part = GEAR;
    armor.gender = GENDER_BOTH;
    armor.rare = 10;
    armor.min_defense = 0;
    armor.max_defense = 0;
    armor.resistance = Resistance{0, 0, 0, 0, 0};
    armor.slots = i;
    armors_.Add(armor, ++external_id);
    if (with_addon) {
      ArmorAddon addon;
      addon.name[ENGLISH] = L"Weapon";
      addon.name[CHINESE] = L"武器";
      addon.name[JAPANESE] = L"武器";
      armor_addons_.Update(addon, external_id);
    }
  }

  // Amulets
  {
    Armor amulet;
    amulet.part = AMULET;
    amulet.gender = GENDER_BOTH;
    amulet.rare = 10;
    amulet.min_defense = 0;
    amulet.max_defense = 0;
    amulet.resistance = Resistance{0, 0, 0, 0, 0};
    amulet.slots = 0;
    armors_.Add(amulet, ++external_id);
    if (with_addon) {
      ArmorAddon addon;
      addon.name[ENGLISH] = L"Amulet";
      addon.name[CHINESE] = L"护石";
      addon.name[JAPANESE] = L"护石";
      armor_addons_.Update(addon, external_id);
    }
  }
}

Armor Data::CreateAmulet(const EffectList &effects, int slots) {
  Armor amulet;
  amulet.part = AMULET;
  amulet.weapon_type = WEAPON_TYPE_BOTH;
  amulet.gender = GENDER_BOTH;
  amulet.rare = 11;
  amulet.min_defense = 0;
  amulet.max_defense = 0;
  amulet.resistance = {0, 0, 0, 0, 0};
  amulet.slots = slots;
  amulet.effects = effects;
  return amulet;
}

int Data::GetMultiplier(const ArmorSet &armor_set, const Arsenal &arsenal) {
  int multiplier = 1;
  for (int i = 0; i < PART_NUM; ++i) {
    if (arsenal.IsTorsoUp(armor_set.ids[i])) {
      multiplier++;
    }
  }
  return multiplier;
}

double Data::EffectScore(const Effect &effect) {
  int armor_count = 0;
  
  for (int i = 0; i < armors_.size(); ++i) {
    for (const Effect &armor_effect : armors_[i].effects) {
      if (armor_effect.id == effect.id && armor_effect.points > 0) {
        armor_count++;
        break;
      }
    }
  }

  double jewel_index = 0;
  for (int i = 0; i < jewels_.size(); ++i) {
    const Jewel &jewel = jewels_[i];
    for (const Effect &jewel_effect : jewel.effects) {
      if (jewel_effect.id == effect.id && jewel_effect.points > 0) {
        jewel_index += static_cast<double>(jewel_effect.points) / jewel.slots
            * std::exp(-jewel.slots * 0.1);
      }
    }
  }
  
  return std::exp(0.1 * jewel_index - 0.3 * effect.points) * armor_count;
}

int Data::GetTotalDefense(const ArmorSet &armor_set, const Arsenal &arsenal) {
  int defense = 0;
  for (int i = 0; i < PART_NUM; ++i) {
    defense += arsenal[armor_set.ids[i]].max_defense;
  }
  return defense;
}

void Data::InternalizeEffectIds() {
  for (int i = 0; i < armors_.size(); ++i) {
    for (Effect &effect : armors_[i].effects) {
      effect.id = skills_.Internalize(effect.id);
    }
  }
  for (int i = 0; i < jewels_.size(); ++i) {
    for (Effect &effect : jewels_[i].effects) {
      effect.id = skills_.Internalize(effect.id);
    }
  }
}

}  // namespace dataset

}  // namespace monster_avengers
