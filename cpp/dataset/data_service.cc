#include <algorithm>
#include <array>
#include <cstdio>
#include <cwchar>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "core/armor_set.h"
#include "dataset.h"

namespace monster_avengers {

namespace dataset {

const std::wstring &Data::GetSkillName(int internal_id, Language language) {
  return skill_addons_[internal_id].name[language];
}

const std::wstring &Data::GetJewelName(int internal_id, Language language) {
  return jewel_addons_[internal_id].name[language];
}

const std::wstring &Data::GetArmorName(int internal_id, Language language) {
  return armor_addons_[internal_id].name[language];
}

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

void Data::PrintArmor(const Armor &armor, int id,
                      int verbose, Language language) {
  char slots[4] = "---";
  for (int i = 0; i < armor.slots; ++i) slots[i] = 'O';
  if (id >= armors_.size()) {
    wprintf(L"[ARMOR %04d] [%s] RARE-%02d  Custom %s\n",
            id, slots, armor.rare,
            StringifyEnum(armor.part).c_str());
  } else {
    const ArmorAddon &addon = armor_addons_[id];
    wprintf(L"[ARMOR %04d] [%s] RARE-%02d  %ls\n",
            id, slots, armor.rare,
            addon.name[language].c_str());
  }
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

void Data::PrintArmor(int id, int verbose, Language language) {
  const Armor &armor = armors_[id];
  PrintArmor(armor, id, verbose, language);
}

void Data::PrintArmorSet(const ArmorSet &armor_set,
                         const Arsenal &arsenal,
                         int verbose,
                         Language language) {
  static std::array<ArmorPart, PART_NUM> ordered_part {
    {GEAR, HEAD, BODY, HANDS, WAIST, FEET, AMULET}};
  
  wprintf(L"__________ Armor Set __________\n");
  for (ArmorPart part : ordered_part) {
    const Armor &armor = arsenal[armor_set.ids[part]];
    
    // Print ID
    wprintf(L"[%04d] ", armor_set.ids[part]);

    // Print Slots
    wchar_t slots_text[4] = L"\u2013\u2013\u2013";
    int occupied = std::accumulate(
        armor_set.jewels[part].begin(),
        armor_set.jewels[part].end(),
        0, [](int sum, int id) {
          return sum + Data::jewel(id).slots;
        });
    for (int i = 0; i < 3; ++i) {
      if (i < occupied) {
        slots_text[i] = L'\u25CF';
      } else if (i < armor.slots) {
        slots_text[i] = L'\u25CB';
      }
    }
    wprintf(L"%ls ", slots_text);

    // Print Name
    wchar_t icon = L'\u23e3';
    if (GEAR == part) {
      icon = L'\u2650';
    } else if (AMULET == part) {
      icon = L'\u2649';
    }
    if (armor_set.ids[part] < armors_.size()) {
      wprintf(L"\u2500\u2565\u2500\u2500 %lc %ls\n", icon,
              armor_addons_[armor_set.ids[part]].name[language].c_str());
    } else {
      wprintf(L"\u2500\u2565\u2500\u2500 %lc %s\n", icon,
              StringifyEnum(part).c_str());
    }

    // Print TorsoUps
    if (arsenal.IsTorsoUp(armor_set.ids[part])) {
      wprintf(L"%15s (Torso-Up)\n", "\u255f\u2500\u2500");
    }

    // Print Jewels
    if (verbose > 0 ) {
      wprintf(L"%16s", "\u2559\u2500\u2500 ");
      // Print Rarity
      wprintf(L"Rare-%02d ", armor.rare);
      for (int jewel_id : armor_set.jewels[part]) {
        wprintf(L"   %ls", jewel_addons_[jewel_id].name[language].c_str());
      }
      wprintf(L"\n");
    }
  }

  // Print Skill Stats
  std::vector<Effect> stats = std::move(GetSkillStats(armor_set, arsenal));
  std::sort(stats.begin(), stats.end(),
            [](const Effect &a, const Effect &b) {
              return a.points > b.points;
            });
  for (const Effect &effect : stats) {
    wprintf(L"%ls(%+d)  ", skill_addons_[effect.id].name[language].c_str(),
            effect.points);
  }
  wprintf(L"\n");
  wprintf(L"\n");
}

}  // namespace dataset

}  // namespace monster_avengers
