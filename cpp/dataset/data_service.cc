#include <algorithm>
#include <array>
#include <cstdio>
#include <cwchar>
#include <unordered_map>
#include <vector>

#include "core/armor_set.h"
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

int Data::GetMultiplier(const ArmorSet &armor_set, const Arsenal &arsenal) {
  int multiplier = 1;
  for (int i = 0; i < PART_NUM; ++i) {
    if (arsenal.IsTorsoUp(armor_set.ids[i])) {
      multiplier++;
    }
  }
  return multiplier;
}

std::vector<Effect> Data::GetSkillStats(
    const ArmorSet &armor_set, const Arsenal &arsenal) {
  std::unordered_map<int, int> stats_map;

  int multiplier = GetMultiplier(armor_set, arsenal);
  
  for (int i = 0; i < PART_NUM; ++i) {
    int armor_id = armor_set.ids[i];
    for (const Effect &effect : arsenal[armor_id].effects) {
      auto iter = stats_map.find(effect.id);
      int points = (BODY == i) ? effect.points * 2 : effect.points;
      if (stats_map.end() != iter) {
        iter->second += points;
      } else {
        stats_map[effect.id] = points;
      }
    }

    // Jewels
    for (int jewel_id : armor_set.jewels[i]) {
      for (const Effect &effect : jewels_[jewel_id].effects) {
        auto iter = stats_map.find(effect.id);
        int points = (BODY == i) ? effect.points * 2 : effect.points;
        if (stats_map.end() != iter) {
          iter->second += points;
        } else {
          stats_map[effect.id] = points;
        }
      }
    }
  }

  std::vector<Effect> stats;
  for (const auto& pair : stats_map) {
    stats.emplace_back();
    stats.back().id = pair.first;
    stats.back().points = pair.second;
  }
  
  return stats;
}

void Data::PrintArmorSet(const ArmorSet &armor_set,
                         const Arsenal &arsenal,
                         int verbose,
                         Language language) {
  static std::array<ArmorPart, PART_NUM> ordered_part {
    {GEAR, HEAD, BODY, HANDS, WAIST, FEET, AMULET}};
  
  wprintf(L"__________ Armor Set __________\n");
  for (ArmorPart part : ordered_part) {
    if (armor_set.ids[part] < armors_.size()) {
      wprintf(L"%-24ls",
              armor_addons_[armor_set.ids[part]].name[language].c_str());
    } else {
      wprintf(L"%-24s", StringifyEnum(part).c_str());
    }
    for (int jewel_id : armor_set.jewels[part]) {
      wprintf(L"(%ls) ", jewel_addons_[jewel_id].name[language].c_str());
    }
    wprintf(L"\n");
  }
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
