#ifndef _MONSTER_AVENGERS_UTILS_FORMATTER_
#define _MONSTER_AVENGERS_UTILS_FORMATTER_

#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <type_traits>
#include "dataset/dataset.h"

using monster_avengers::dataset::Arsenal;
using monster_avengers::dataset::Data;
using monster_avengers::dataset::ArmorSet;
using monster_avengers::dataset::JewelSet;

namespace monster_avengers {

class ArmorSetFormatter {
 public:
  ArmorSetFormatter(const Arsenal *arsenal)
      : arsenal_(arsenal) {}

  virtual void Format(const ArmorSet &armor_set,
                      std::ostream *out) const = 0;

  virtual void Prepend (std::ostream *out) const {}
  virtual void Postpend (std::ostream *out) const {}
  virtual void AddDelimiter(std::ostream *out) const {}

  inline const Arsenal &arsenal() const {
    return *arsenal_;
  }

  std::string StringBatchFormat(const std::vector<ArmorSet> &armor_sets,
                                int max_output = -1) const {
    std::stringstream result;

    Prepend(&result);
    for (int i = 0; i < armor_sets.size(); ++i) {
      const ArmorSet &armor_set = armor_sets[i];
      Format(armor_set, &result);
      if (0 < max_output && i + 1 >= max_output) {
        break;
      }
      if (i < armor_sets.size() - 1) {
        AddDelimiter(&result);
      }
    }
    Postpend(&result);
    return result.str();
  }
  
 private:
  const Arsenal *arsenal_;
};

// Note that the Dex's Json formaterr does not externalize
// the ids, since it mainly serves the internalized dex database application.
class DexJsonFormatter : public ArmorSetFormatter {
 public:
  DexJsonFormatter(const Arsenal *arsenal) :
      ArmorSetFormatter(arsenal) {}

  void Prepend(std::ostream *out) const override {
    (*out) << "[";
  }

  void Postpend(std::ostream *out) const override {
    (*out) << "]";
  }

  void AddDelimiter(std::ostream *out) const override {
    (*out) << ", ";
  }

  void Format(const ArmorSet &armor_set,
              std::ostream *out) const override {
    // Turn on for debug.
    // Data::PrintArmorSet(armor_set, arsenal(), 1, JAPANESE);
    
    (*out) << "{";
    WriteGear(armor_set.ids[GEAR], armor_set.jewels[GEAR], out);
    (*out) << ", ";
    WriteArmor("head", armor_set.ids[HEAD],
               armor_set.jewels[HEAD], out);
    (*out) << ", ";
    WriteArmor("body", armor_set.ids[BODY],
               armor_set.jewels[BODY], out);
    (*out) << ", ";
    WriteArmor("hands", armor_set.ids[HANDS],
               armor_set.jewels[HANDS], out);
    (*out) << ", ";
    WriteArmor("waist", armor_set.ids[WAIST],
               armor_set.jewels[WAIST], out);
    (*out) << ", ";
    WriteArmor("feet", armor_set.ids[FEET],
               armor_set.jewels[FEET], out);
    (*out) << ", ";
    WriteAmulet(armor_set.ids[AMULET], armor_set.jewels[AMULET], out);
    (*out) << "}";
  }

 private:
  void WriteArmor(const std::string &name, int armor_id,
                  const JewelSet &jewel_set, std::ostream *out) const {
    (*out) << "\"" << name << "\"" << ": {";
    (*out) << "\"id\": " << armor_id << ", ";
    (*out) << "\"jewels\": ";
    WriteArray(jewel_set, out, WriteJewelID);
    (*out) << "}";
  }

  void WriteAmulet(int id, const JewelSet &jewel_set,
                   std::ostream *out) const {
    (*out) << "\"amulet\": {\"effects\": ";
    WriteArray(arsenal()[id].effects, out, WriteEffect);
    (*out) << ", \"jewels\": ";
    WriteArray(jewel_set, out, WriteJewelID);
    (*out) << ", \"slots\": " << arsenal()[id].slots << "}";
  }

  void WriteGear(int id, const JewelSet &jewel_set,
                 std::ostream *out) const {
    (*out) << "\"gear\": {\"jewels\": ";
    WriteArray(jewel_set, out, WriteJewelID);
    (*out) << ", \"slots\": " << arsenal()[id].slots << "}";
  }
  
  template <typename Iteratable>
  static void WriteArray(
      const Iteratable array, std::ostream *out,
      std::function<void(const decltype(array[0])&, std::ostream*)> callback) {
    (*out) << "[";
    for (int i = 0; i < array.size(); ++i) {
      callback(array[i], out);
      if (i < array.size() - 1) {
        (*out) << ", ";
      }
    }
    (*out) << "]";
  }

  static void WriteJewelID(const int &id, std::ostream* out) {
    // Note(breakds): This is a hack as the application does not
    //                directly use the internal ID for jewels, since
    //                there can be multiple records for a jewel in the
    //                database (different produce methods).
    (*out) << Data::jewels().Externalize(id) - 1;
  }

  static void WriteEffect(const Effect &effect, std::ostream* out) {
    (*out) << "{\"id\": " << effect.id << ", \"points\": " << effect.points << "}";
  }
  
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
    (*out) << ")\n";
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

  static void WriteJewels(const JewelSet &jewels, std::ostream *out) {
    (*out) << "(";
    bool first = true;
    for (int jewel_id : jewels) {
      int external_id = Data::jewels().Externalize(jewel_id);
      if (!first) {
        (*out) << " ";
      } else {
        first = false;
      }
      (*out) << external_id;
    }
    (*out) << ")";
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
    WriteJewels(jewels, out);
    (*out) << ")";
  }
};

}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_UTILS_FORMATTER_
