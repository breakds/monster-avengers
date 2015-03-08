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
#include "utils/output_specs.h"

namespace monster_avengers {

  void OutputArmorSet(const DataSet &data, 
                      const ArmorSet &armor_set, 
                      const JewelSolver &solver) {
    int defense = 0;
    std::vector<Effect> effects;
    const std::array<int, PART_NUM> &ids = armor_set.ids;
    int multiplier = 1;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor &armor = data.armor(ids[i]);
      if (armor.TorsoUp()) {
        multiplier++;
      }
    }
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
        if (BODY == armor.part && multiplier > 1) {
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
      JewelSolver::JewelPlan jewel_plan = solver.Solve(jewel_key, multiplier);
      for (auto item : jewel_plan.second) {
        wprintf(L"%d x %ls{BODY}  ", item.second, 
                data.jewel(item.first).name.c_str());
      }
      for (auto item : jewel_plan.first) {
        wprintf(L"%d x %ls  ", item.second, 
                data.jewel(item.first).name.c_str());
      }
      wprintf(L"\n");
      if (++plan_count >= 5) {
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

    ArmorSetFormatter(const DataSet *data,
                      const Query &query)
      : output_stream_(),
        solver_(*data, query.effects), 
        data_(data) {}
    
    void Format(const ArmorSet &armor_set) {
      (*output_stream_) << ArmorResult(*data_, solver_, armor_set) << "\n";
    }

    void Text(const ArmorSet &armor_set) const {
      ArmorResult result(*data_, solver_, armor_set);
      wprintf(L"---------- ArmorSet (defense %d) ----------\n", 
              result.defense);
      WriteGear(result.gear);
      WriteArmor(HEAD, result.head);
      WriteArmor(BODY, result.body);
      WriteArmor(HANDS, result.hands);
      WriteArmor(WAIST, result.waist);
      WriteArmor(FEET, result.feet);
      WriteAmulet(result.amulet);
      
      for (const JewelPlan &plan : result.plans) {
        wprintf(L"Jewel Plan:");
        for (const JewelPair &pair : plan.body_plan) {
          wprintf(L" | %ls[BODY] x %d", 
                  pair.name.c_str(),
                  pair.quantity);
        }
        for (const JewelPair &pair : plan.plan) {
          wprintf(L" | %ls x %d", 
                  pair.name.c_str(),
                  pair.quantity);
        }
        wprintf(L" |\n");
        for (const SummaryItem &item : plan.summary) {
          wprintf(L"%ls(%d)  ", item.name.c_str(), item.points);
        }
        wprintf(L"\n");
      }
      wprintf(L"\n");
    }
    
  private:

    void WriteGear(const PackedArmor &gear) const {
      wprintf(L"[ GEAR ] [%s] [Rare ??] %ls\n", 
              HoleText(gear.holes).c_str(),
              gear.name.c_str());
    }

    void WriteArmor(ArmorPart part, const PackedArmor &armor) const {
      wprintf(L"[%s] [%s] [Rare %02d] %ls", 
              PartText(part).c_str(),
              HoleText(armor.holes).c_str(),
              armor.rare,
              armor.name.c_str());
      if (L"true" == armor.torso_up) {
        wprintf(L"(%ls)",
                data_->skill_system(data_->torso_up_id).name.c_str());
      }
      wprintf(L"        | Material:");
      for (const LanguageText &material : armor.material) {
        wprintf(L" %ls", material.c_str());
      }
      wprintf(L"\n");
    }

    void WriteAmulet(const PackedArmor &amulet) const {
      wprintf(L"[AMULET] [%s] [Rare ??] %ls\n", 
              HoleText(amulet.holes).c_str(),
              amulet.name.c_str());
    }
    
    std::string HoleText(int holes) const {
      switch (holes) {
      case 1: return "O--";
      case 2: return "OO-";
      case 3: return "OOO";
      default: return "---";
      }
    }

    std::string PartText(ArmorPart part) const {
      switch (part) {
      case HEAD: return " HEAD ";
      case BODY: return " BODY ";
      case HANDS: return " ARMS ";
      case WAIST: return " LEGS ";
      case FEET: return " FEET ";
      default: return "---";
      }
    }


    std::unique_ptr<std::wofstream> output_stream_;
    const JewelSolver solver_;
    const DataSet *data_;
  };

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ARMOR_SET_

