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
  class ArmorSetFormatter {
  public:
    ArmorSetFormatter(const std::string file_name, 
                      const DataSet *data,
                      const Query &query)
      : to_screen_("" == file_name),
        solver_(*data, query.effects), 
        data_(data) {
      if (!to_screen_) {
        output_stream_.reset(new std::wofstream(file_name));
        if (!output_stream_->good()) {
          Log(ERROR, L"error while opening %s.", file_name.c_str());
          exit(-1);
        }
        output_stream_->imbue(std::locale("en_US.UTF-8"));
      }
    }

    void operator()(const ArmorSet &armor_set) {
      if (to_screen_) {
        ToScreen(armor_set);
      } else {
        ToFile(armor_set);
      }
    }

  private:
    // Output to specified file.
    void ToFile(const ArmorSet &armor_set) {
      (*output_stream_) << ArmorResult(*data_, solver_, armor_set) << "\n";
    }

    // Output to screen.
    void ToScreen(const ArmorSet &armor_set) const {
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


    bool to_screen_;
    std::unique_ptr<std::wofstream> output_stream_;
    const JewelSolver solver_;
    const DataSet *data_;
  };


  class ExploreFormatter {
  public:
    ExploreFormatter(const std::string &file_name) 
      : to_screen_("" == file_name) {
      if (!to_screen_) {
        output_stream_.reset(new std::wofstream(file_name));
        if (!output_stream_->good()) {
          Log(ERROR, L"error while opening %s.", file_name.c_str());
          exit(-1);
        }
        output_stream_->imbue(std::locale("en_US.UTF-8"));
      }
    }

    void Push(int skill_id, bool pass, const LanguageText &name, 
              double duration) {
      if (to_screen_) {
        wprintf(L"%.4lf sec, (%03d) %ls %s\n",
                duration,
                skill_id,
                name.c_str(),
                pass ? "[PASS]" : "[fail]");
      } else {
        (*output_stream_) << "(" << skill_id << " "
                          << (pass ? ":PASS" : ":FAIL") << ")\n";
        output_stream_->flush();
      }
    }
    
  private:
    bool to_screen_;
    std::unique_ptr<std::wofstream> output_stream_;
  };

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ARMOR_SET_

