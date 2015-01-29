#ifndef _MONSTER_AVENGERS_ARMOR_SET_
#define _MONSTER_AVENGERS_ARMOR_SET_

#include <cstdio>
#include <cwchar>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>

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

  class ArmorSetFormatter {
  public:
    ArmorSetFormatter(const std::string file_name, 
                      const DataSet *data,
                      const Query &query)
      : output_stream_(new std::wofstream(file_name)),
        query_(query), solver_(*data, query.effects), 
        data_(data) {
      if (!output_stream_->good()) {
        Log(ERROR, L"error while opening %s.", file_name.c_str());
        exit(-1);
      }
      output_stream_->imbue(std::locale("en_US.UTF-8"));
    }

    void Format(const ArmorSet &armor_set) {
      int defense = 0;
      std::unordered_map<int, int> effects;
      (*output_stream_) << "(";
      for (int part = 0; part < PART_NUM; ++part) {
        int id = armor_set.ids[part];
        const Armor &armor = (id < data_->armors().size()) ?
          data_->armor(id) : query_.amulets[id - data_->armors().size()];
        FormatArmor(armor, static_cast<ArmorPart>(PART_NUM - part - 1));
        defense += armor.defense;
        for (const Effect &effect : armor.effects) {
          auto it = effects.find(effect.skill_id);
          if (effects.end() == it) {
            effects[effect.skill_id] = effect.points;
          } else {
            it->second += effect.points;
          }
        }
      }

      (*output_stream_) << " :jewel-plans (";
      int jewel_plan_count = 0;
      for (const Signature &jewel_key : armor_set.jewel_keys) {
        if (jewel_plan_count < MAX_JEWEL_PLAN) {
          std::unordered_map<int, int> jewel_plan_effects = effects;
          (*output_stream_) << "(:jewel-plan (";
          for (auto item : solver_.Solve(jewel_key)) {
            const Jewel &jewel = data_->jewel(item.first);
            (*output_stream_) << "(\""
                              << jewel.name.c_str()
                              << "\" " << item.second << ") ";
            for (const Effect &effect : jewel.effects) {
              auto it = jewel_plan_effects.find(effect.skill_id);
              if (jewel_plan_effects.end() == it) {
                jewel_plan_effects[effect.skill_id] = effect.points;
              } else {
                it->second += effect.points;
              }
            }
          }
          (*output_stream_) << ") :active ";
          FormatActive(jewel_plan_effects);
        }
        (*output_stream_) << ") ";
        jewel_plan_count++;
      }
      (*output_stream_) << ") ";
      (*output_stream_) << " :defense " << defense;
      (*output_stream_) << ")\n";
    }
    
  private:
    void FormatArmor(const Armor &armor, ArmorPart part) {
      switch (part) {
      case HEAD: (*output_stream_) << ":helm "; break;
      case BODY: (*output_stream_) << ":body "; break;
      case HANDS: (*output_stream_) << ":hands "; break;
      case WAIST: (*output_stream_) << ":waist "; break;
      case FEET: (*output_stream_) << ":feet "; break;
      case AMULET: (*output_stream_) << ":amulet "; break;
      case GEAR: (*output_stream_) << ":gear "; break;
      default: Log(ERROR, L"FormatArmor: no such armor part %d", part);
      }
      (*output_stream_) << "(";
      (*output_stream_) << ":name " << "\"" << armor.name << "\" ";
      (*output_stream_) << ":holes " << armor.holes << " ";
      if (AMULET == part) {
        (*output_stream_) << ":effects ";
        FormatEffects(armor.effects);
      } else if (GEAR != part) {
        (*output_stream_) << ":material ";
        FormatMaterial(armor.material);
      }        
      (*output_stream_) << ") ";
    }

    void FormatEffects(const std::vector<Effect> &effects) {
      (*output_stream_) << "(";
      for (const Effect &effect : effects) {
        (*output_stream_) << "("
                         << data_->skill_system(effect.skill_id).name
                         << " " << effect.points << ")";
      }
      (*output_stream_) << ") ";
    }

    void FormatMaterial(const std::vector<std::wstring> &material) {
      (*output_stream_) << "(";
      for (const std::wstring item : material) {
        (*output_stream_) << "\"" << item << "\" ";
      }
      (*output_stream_) << ") ";
    }

    void FormatActive(const std::unordered_map<int, int> &effects) {
      (*output_stream_) << " (";
      for (auto &effect : effects) {
        const SkillSystem &skill_system = data_->skill_system(effect.first);
        int active_points = 0;
        std::wstring active_name = L"";
        if (effect.second > 0) {
          for (const Skill &skill : skill_system.skills) {
            if (effect.second > 0) {
              if (effect.second >= skill.points && 
                  skill.points > active_points) {
                active_points = skill.points;
                active_name = skill.name;
              }
            } else if (effect.second <= skill.points && 
                       skill.points < active_points) {
              active_points = skill.points;
              active_name = skill.name;
            }
          }
        }
        if (!active_name.empty()) {
          (*output_stream_) << "\"" << active_name << "\" ";
        }
      }
      (*output_stream_) << ") ";
    }

    int GetDefense(const ArmorSet &armor_set) {
      int defense = 0;
      for (int part = 0; part < PART_NUM; ++part) {
        defense += data_->armor(armor_set.ids[part]).defense;
      }
      return defense;
    }

    

    std::unique_ptr<std::wofstream> output_stream_;
    const Query query_;
    const JewelSolver solver_;
    const DataSet *data_;
  };
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ARMOR_SET_

