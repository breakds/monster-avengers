#ifndef _MONSTER_AVENGERS_UTILS_FORMATTER_
#define _MONSTER_AVENGERS_UTILS_FORMATTER_

#include <iostream>
#include <ostream>
#include <sstream>
#include "dataset/dataset.h"


using monster_avengers::dataset::Arsenal;
using monster_avengers::dataset::Data;

namespace monster_avengers {

class ArmorSetFormatter {
 public:
  ArmorSetFormatter(const Arsenal *arsenal)
      : arsenal_(arsenal) {}
  
  virtual void Format(const ArmorSet &armor_set,
                      std::ostream *out) const = 0;

  inline const Arsenal &arsenal() const {
    return *arsenal_;
  }

  std::string StringBatchFormat(const std::vector<ArmorSet> &armor_sets,
                                int max_output = -1) const {
    std::stringstream result;

    int count = 0;
    for (const ArmorSet &armor_set : armor_sets) {
      Format(armor_set, &result);
      if (0 < max_output && (++count) >= max_output) {
        break;
      }
    }
    return result.str();
  }
  
 private:
  const Arsenal *arsenal_;
};

class DexFormatter : public ArmorSetFormatter {
 public:
  DexFormatter(const Arsenal *arsenal) :
      ArmorSetFormatter(arsenal) {}

  void Format(const ArmorSet &armor_set,
              std::ostream *out) const override {
    (*out) << "(";
    for (int part = 0; part < PART_NUM; ++part) {
      WriteArmor(armor_set.ids[part],
                 armor_set.jewels[part],
                 out);
    }
  }

 private:
  template <typename ElementType>
  static void WriteVector(const std::vector<ElementType> &input,
                          std::ostream *out) {
    (*out) << "(";
    for (const ElementType &element : input) {
      (*out) << element;
      (*out) << " ";
    }
    (*out) << ")";
  }

  static void WriteEffects(const std::vector<Effect> &effects,
                           std::ostream *out) {
    std::vector<int> effect_buffer;
    for (const Effect &effect : effects) {
      effect_buffer.push_back(Data::skills().Externalize(effect.id));
      effect_buffer.push_back(effect.points);
    }
    WriteVector(effect_buffer, out);    
  }
  
  void WriteArmor(int armor_id, const JewelSet &jewels,
                  std::ostream *out) const {
    const Armor &armor = arsenal()[armor_id];
    if (GEAR == armor.part) {
      (*out) << ":GEAR (";
      (*out) << armor.slots;
    } else if (AMULET == armor.part) {
      (*out) << ":AMULET (";
      WriteEffects(armor.effects, out);
      // Otuput effects as (id, points, id, points ..)

    } else {
      // Write the part name as the attribute name
      switch (armor.part) {
        case HEAD: (*out) << ":HEAD ("; break;
        case HANDS: (*out) << ":ARMS ("; break;
        case WAIST: (*out) << ":WAIST ("; break;
        case FEET: (*out) << ":LEGS ("; break;
        case BODY: (*out) << ":BODY ("; break;
        default: CHECK(false);
      }

      // Write the external armor id
      (*out) << Data::armors().Externalize(armor_id);
    }

    // Write jewel ids and close the parenthesis
    (*out) << " ";
    WriteVector(jewels, out);
    (*out) << ")\n";
  }
};

}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_UTILS_FORMATTER_
