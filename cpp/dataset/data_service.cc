#include <cstdio>
#include <cwchar>
#include "dataset.h"


namespace monster_avengers {

namespace dataset {

void Data::PrintSkill(int id, int verbose, Language language) {
  const SkillSystem &skill = skills_[id];
  const SkillSystemAddon &addon = skill_addons_[id];
  wprintf(L"[SKILL %03d] %ls", id, addon.name[language].c_str());
  if (verbose >= 1) {
    wprintf(L":");
    for (int i = 0; i < skill.skills.size(); ++i) {
      wprintf(L"  [%d %ls]", skill.skills[i].points,
              addon.skill_names[i][language].c_str());
    }
  }
  wprintf(L"\n");
}

void Data::PrintJewel(int id, int verbose, Language language) {
  const Jewel &jewel = jewels_[id];
  const JewelAddon &addon = jewel_addons_[id];
  wprintf(L"[JEWEL %03d] %ls", id, addon.name[language].c_str());
  if (verbose >= 1) {
    wprintf(L":");
    for (const Effect &effect : jewel.effects) {
      wprintf(L"   [%03d]%ls %d", effect.id,
              skill_addons_[effect.id].name[language].c_str(),
              effect.points);
    }
  }
  wprintf(L"\n");
}

void Data::PrintArmor(int id, int verbose, Language language) {
  const Armor &armor = armors_[id];
  const ArmorAddon &addon = armor_addons_[id];
  char slots[4] = "---";
  for (int i = 0; i < armor.slots; ++i) slots[i] = 'O';
  wprintf(L"[ARMOR %04d] [%s] RARE-%02d  %ls\n",
          id, slots, armor.rare,
          addon.name[language].c_str());
  if (verbose >= 1) {
    wprintf(L"             [Part: %s]\n", StringifyEnum(armor.part).c_str());
    wprintf(L"             [Type: %s]\n",
            StringifyEnum(armor.weapon_type).c_str());
    wprintf(L"             [Gender: %s]\n",
            StringifyEnum(armor.gender).c_str());
    wprintf(L"             [Def(Min): %d]\n", armor.min_defense);
    wprintf(L"             [Def(Max): %d]\n", armor.max_defense);
    wprintf(L"             [Resist: F(%d) | T(%d) | D(%d) | W(%d) | I(%d)]\n",
            armor.resistance.fire,
            armor.resistance.thunder,
            armor.resistance.dragon,
            armor.resistance.water,
            armor.resistance.ice);
    for (const Effect &effect : armor.effects) {
      wprintf(L"             [%03d]%ls %d\n",
              effect.id,
              skill_addons_[effect.id].name[language].c_str(),
              effect.points);
    }
  }
}



}  // namespace dataset

}  // namespace monster_avengers
