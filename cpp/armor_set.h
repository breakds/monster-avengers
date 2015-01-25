#ifndef _MONSTER_AVENGERS_ARMOR_SET_
#define _MONSTER_AVENGERS_ARMOR_SET_

namespace monster_avengers {

  const int MAX_JEWEL_PLAN = 5;

  struct ArmorSet {
    std::array<int, PART_NUM> ids;
    std::vector<Signature> jewel_keys;
  };
  
  void OutputArmorSet(const DataSet &data, 
                      const ArmorSet &armor_set, 
                      const Query &query,
                      const JewelSolver &solver) {
    wprintf(L"---------- Armor Set ----------\n");
    int defense = 0;
    std::vector<Effect> effects;
    const std::array<int, PART_NUM> &ids = armor_set.ids;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor &armor = (ids[i] < data.armors().size()) ?
        data.armor(ids[i]) : query.amulets[ids[i] - data.armors().size()];
      defense += data.armor(ids[i]).defense;
      wprintf(L"[");
      for (int j = 0; j < 3; ++j) {
        if (j < armor.holes) {
          wprintf(L"O");
        } else {
          wprintf(L"-");
        }
      }
      if (GEAR == armor.part) {
        wprintf(L"] [Rare ??] ??? %ls (%d)\n", 
                armor.name.c_str(), ids[i]);
      } else {
        wprintf(L"] [Rare %02d] %s %ls (%d)\n", 
                armor.rare,
                (MELEE == armor.type) ? "--H" : ")->", 
                armor.name.c_str(), ids[i]);
      }
      for (const Effect &effect : armor.effects) {
        auto it = std::find_if(effects.begin(), effects.end(),
                               [&effect](const Effect& x) {
                                 return x.skill_id == effect.skill_id;
                               });
        if (effects.end() == it) {
          effects.push_back(effect);
        } else {
          it->points += effect.points;
        }
      }
    }
    wprintf(L"Defense: %d\n", defense);
    for (const Effect &effect : effects) {
      wprintf(L"%ls(%d)  ", 
              data.skill_system(effect.skill_id).name.c_str(),
              effect.points);
    }
    wprintf(L"\n");

    int plan_count = 0;
    for (const Signature &jewel_key : armor_set.jewel_keys) {
      wprintf(L"Jewel Plan:    ");
      for (auto item : solver.Solve(jewel_key)) {
        wprintf(L"%d x %ls  ", item.second, 
                data.jewel(item.first).name.c_str());
      }
      wprintf(L"\n");
      if (++plan_count >= MAX_JEWEL_PLAN) {
        break;
      }
    }
    wprintf(L"\n");
  }

  void PrettyPrintArmorSet(const DataSet &data, 
			   const ArmorSet &armor_set, 
			   const Query &query,
			   const JewelSolver &solver) {
    int defense = 0;
    std::vector<Effect> effects;
    const std::array<int, PART_NUM> &ids = armor_set.ids;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor &armor = (ids[i] < data.armors().size()) ?
        data.armor(ids[i]) : query.amulets[ids[i] - data.armors().size()];
      defense += data.armor(ids[i]).defense;
      wprintf(L"[");
      for (int j = 0; j < 3; ++j) {
        if (j < armor.holes) {
          wprintf(L"O");
        } else {
          wprintf(L"-");
        }
      }
      if (GEAR == armor.part) {
        wprintf(L"] [Rare ??] ??? %ls\n", 
                armor.name.c_str());
      } else if (AMULET == armor.part) {
        wprintf(L"] [Rare ??] ??? %ls    ",
                armor.name.c_str());
	for (const Effect &effect : armor.effects) {
	  wprintf(L"%ls(%d) ", 
		  data.skill_system(effect.skill_id).name.c_str(),
		  effect.points);
	}
	wprintf(L"\n");
      } else {
        wprintf(L"] [Rare %02d] %s %ls (id: %d)     Require:",
                armor.rare,
                (MELEE == armor.type) ? "--H" : ")->", 
		armor.name.c_str(), ids[i]);

        for (const std::wstring &material_name : armor.material) {
          wprintf(L"  %ls", material_name.c_str());
        }
        wprintf(L"\n");
      }
      for (const Effect &effect : armor.effects) {
        auto it = std::find_if(effects.begin(), effects.end(),
                               [&effect](const Effect& x) {
                                 return x.skill_id == effect.skill_id;
                               });
        if (effects.end() == it) {
          effects.push_back(effect);
        } else {
          it->points += effect.points;
        }
      }
    }
    wprintf(L"Defense: %d\n", defense);
    for (const Effect &effect : effects) {
      wprintf(L"%ls(%d)  ", 
              data.skill_system(effect.skill_id).name.c_str(),
              effect.points);
    }
    wprintf(L"\n");
    int plan_count = 0;
    for (const Signature &jewel_key : armor_set.jewel_keys) {
      wprintf(L"Jewel Plan:    ");
      for (auto item : solver.Solve(jewel_key)) {
        wprintf(L"%d x %ls  ", item.second, 
                data.jewel(item.first).name.c_str());
      }
      wprintf(L"\n");
      if (++plan_count >= MAX_JEWEL_PLAN) {
        break;
      }
    }
    wprintf(L"\n");
  }


}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ARMOR_SET_

