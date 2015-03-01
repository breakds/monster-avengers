#ifndef _MONSTER_AVENGERS_ARMOR_SET_
#define _MONSTER_AVENGERS_ARMOR_SET_

#include <cstdio>
#include <cwchar>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>

#include "lisp/lisp_object.h"

namespace monster_avengers {

  const int MAX_JEWEL_PLAN = 5;

  struct ArmorSet {
    std::array<int, PART_NUM> ids;
    std::vector<Signature> jewel_keys;
  };
  
  void OutputArmorSet(const DataSet &data, 
                      const ArmorSet &armor_set, 
                      const JewelSolver &solver) {
    wprintf(L"---------- Armor Set ----------\n");
    int defense = 0;
    std::vector<Effect> effects;
    const std::array<int, PART_NUM> &ids = armor_set.ids;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor &armor = data.armor(ids[i]);
      defense += data.armor(ids[i]).max_defense;
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
        if (armor.multiplied) {
          const Armor &base = data.armor(armor.base);
          wprintf(L"] [Rare %02d] %s %ls (%d) ",
                  base.rare,
                  (MELEE == base.type) ? "--H" : ")->", 
                  base.name.c_str(), armor.base);
          for (auto item : armor.jewels) {
            wprintf(L"%d x %ls ", item.second,
                    data.jewel(item.first).name.c_str());
          }
          wprintf(L"\n");
        } else {
          wprintf(L"] [Rare %02d] %s %ls (%d) %ls\n", 
                  armor.rare,
                  (MELEE == armor.type) ? "--H" : ")->", 
                  armor.name.c_str(), ids[i],
                  data.ProvidesTorsoUp(ids[i]) ?
                  data.skill_system(data.torso_up_id).name.c_str() : L"");
        }
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
			   const JewelSolver &solver) {
    int defense = 0;
    std::vector<Effect> effects;
    const std::array<int, PART_NUM> &ids = armor_set.ids;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor &armor = data.armor(ids[i]);
      defense += data.armor(ids[i]).max_defense;
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

        // for (const std::wstring &material_name : armor.material) {
        //   wprintf(L"  %ls", material_name.c_str());
        // }
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

  

  class ArmorSetFormatter {
  public:
    ArmorSetFormatter(const std::string file_name, 
                      const DataSet *data,
                      const Query &query)
      : output_stream_(new std::wofstream(file_name)),
        solver_(*data, query.effects), 
        data_(data) {
      if (!output_stream_->good()) {
        Log(ERROR, L"error while opening %s.", file_name.c_str());
        exit(-1);
      }
      output_stream_->imbue(std::locale("en_US.UTF-8"));
    }

    void Format(const ArmorSet &armor_set) {
      lisp::Object output = lisp::Object::Struct();
      int defense = 0;
      std::unordered_map<int, int> effects;
      
      int torso_multiplier = 1;
      for (int i = HEAD; i < PART_NUM; ++i) {
        int id = armor_set.ids[i];
        if (data_->ProvidesTorsoUp(id)) torso_multiplier++;
      }

      for (int i = 0; i < PART_NUM; ++i) {
        ArmorPart part = static_cast<ArmorPart>(PART_NUM - i - 1);
        int id = armor_set.ids[i];
        const Armor &armor = data_->armor(id);
	output.Set(PartName(part),
		   GetArmorObject(data_, armor, part, id));
        defense += armor.max_defense;
        for (const Effect &effect : armor.effects) {
          int points = part == BODY ? 
            effect.points * torso_multiplier : effect.points;
          auto it = effects.find(effect.skill_id);
          if (effects.end() == it) {
            effects[effect.skill_id] = points;
          } else {
            it->second += points;
          }
        }
      }

      output.Set("defense", defense);

      output.Set("jewel-plans", lisp::Object::List());

      int jewel_plan_count = 0;
      for (const Signature &jewel_key : armor_set.jewel_keys) {
        if (jewel_plan_count < MAX_JEWEL_PLAN) {
	  lisp::Object jewel_plan_object = lisp::Object::Struct();
	  jewel_plan_object.Set("plan", lisp::Object::List());
          std::unordered_map<int, int> jewel_plan_effects = effects;
          for (auto item : solver_.Solve(jewel_key)) {
	    lisp::Object plan_object = lisp::Object::Struct();
            const Jewel &jewel = data_->jewel(item.first);
	    plan_object.Set("name", GetLanguageText(jewel.name));
	    plan_object.Set("quantity", item.second);
	    jewel_plan_object["plan"].Push(std::move(plan_object));
            for (const Effect &effect : jewel.effects) {
              auto it = jewel_plan_effects.find(effect.skill_id);
              if (jewel_plan_effects.end() == it) {
                jewel_plan_effects[effect.skill_id] = 
                  effect.points * item.second;
              } else {
                it->second += effect.points * item.second;
              }
            }
          }
	  jewel_plan_object.Set("active", GetActiveObject(jewel_plan_effects));
          output["jewel-plans"].Push(std::move(jewel_plan_object));
        }
        jewel_plan_count++;
      }
      
      (*output_stream_) << output << "\n";
    }
    
  private:
    std::string PartName(ArmorPart part) {
      switch (part) {
      case HEAD: return "head"; 
      case BODY: return "body";
      case HANDS: return "hands";
      case WAIST: return "waist";
      case FEET: return "feet";
      case AMULET: return "amulet";
      case GEAR: return "gear";
      default: Log(ERROR, L"FormatArmor: no such armor part %d", part); exit(-1); break;
      }
    }
    
    lisp::Object GetLanguageText(const LanguageText &text) {
      lisp::Object result = lisp::Object::Struct();
      result["en"] = text.en;
      result["jp"] = text.jp;
      return result;
    }
    
    lisp::Object GetArmorObject(const DataSet *data, const Armor &armor, 
                              ArmorPart part, int id) {
      lisp::Object armor_object = lisp::Object::Struct();
      armor_object["name"] = GetLanguageText(armor.name);
      armor_object["holes"] = armor.holes;
      armor_object["id"] = std::to_wstring(id);
      armor_object["max-defense"] = armor.max_defense;
      armor_object["min-defense"] = armor.min_defense;
      armor_object.Set("resistence", armor.resistence.ToObject());
      armor_object.Set("torsoup",
                       data_->ProvidesTorsoUp(id) ? 
                       std::wstring(L"true") : 
                       std::wstring(L"false"));
      if (AMULET == part) {
        armor_object.Set("effects", GetEffectsObject(armor.effects));
      } else if (GEAR != part) {
        armor_object.Set("material", 
                         GetMaterialObject(data, armor.material));
        armor_object.Set("rare", armor.rare);
      } 
      
      if (armor.multiplied) {
        const Armor &base_armor = data_->armor(armor.base);
        armor_object["holes"] = base_armor.holes;
        int stuffed = 0;
        if (!armor.jewels.empty()) {
          armor_object.Set("jewels", lisp::Object::List());
          for (const auto &item : armor.jewels) {
            const Jewel &jewel = data_->jewel(item.first);
            lisp::Object jewel_object = lisp::Object::Struct();
            jewel_object.Set("name", GetLanguageText(jewel.name));
            jewel_object.Set("quantity", item.second);
            stuffed += jewel.holes * item.second;
            armor_object["jewels"].Push(std::move(jewel_object));
          }
          armor_object.Set("stuffed", stuffed);
        }
      }

      return armor_object;
    }

    lisp::Object GetEffectsObject(const std::vector<Effect> &effects) {
      lisp::Object result = lisp::Object::List();
      for (const Effect &effect : effects) {
	lisp::Object effect_object = lisp::Object::Struct();
	effect_object.Set("name", 
                          GetLanguageText(data_->skill_system(effect.skill_id).name));
	effect_object.Set("points", effect.points); 
	result.Push(std::move(effect_object));
      }
      return result;
    }

    lisp::Object GetMaterialObject(const DataSet *data, 
                                 const std::vector<int> &material) {
      lisp::Object result = lisp::Object::List();
      for (const int &item_id : material) {
	result.Push(GetLanguageText(data->ItemName(item_id)));
      }
      return result;
    }

    lisp::Object GetActiveObject(const std::unordered_map<int, int> &effects) {
      lisp::Object active_object = lisp::Object::List();
      for (auto &effect : effects) {
        const SkillSystem &skill_system = data_->skill_system(effect.first);
        int active_points = 0;
        lisp::Object active_name;
        if (effect.second > 0) {
          for (const Skill &skill : skill_system.skills) {
            if (effect.second > 0) {
              if (effect.second >= skill.points && 
                  skill.points > active_points) {
                active_points = skill.points;
                active_name = GetLanguageText(skill.name);
              }
            } else if (effect.second <= skill.points && 
                       skill.points < active_points) {
              active_points = skill.points;
              active_name = GetLanguageText(skill.name);
            }
          }
        }
        if (0 != active_points) {
	  active_object.Push(std::move(active_name));
        }
      }
      return active_object;
    }

    std::unique_ptr<std::wofstream> output_stream_;
    const JewelSolver solver_;
    const DataSet *data_;
  };
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ARMOR_SET_

