#ifndef _MONSTER_AVENGERS_DATA_LOADER_
#define _MONSTER_AVENGERS_DATA_LOADER_

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include <unordered_map>

#include "supp/helpers.h"
#include "lisp/lisp_object.h"
#include "lisp/reader.h"

#include "language_text.h"
#include "effect.h"
#include "skill.h"
#include "jewel.h"
#include "item.h"
#include "armor.h"

namespace monster_avengers {

class LoaderCore {
 public:
  virtual ~LoaderCore() {}
  virtual std::vector<SkillSystem> LoadSkillSystems() = 0;
  virtual std::vector<Jewel> LoadJewels() = 0;
  virtual std::vector<Item> LoadItems() = 0;
  virtual std::vector<Armor> LoadArmors() = 0;
};


// ---------- Standard Loader Core ----------
class StandardLoaderCore : public LoaderCore {
 public:
  StandardLoaderCore(const std::string &data_folder) 
      : data_folder_(data_folder) {}

  virtual std::vector<SkillSystem> LoadSkillSystems() override {
    return lisp::ReadList<SkillSystem>(data_folder_ + "/skills.lisp");
  }

  virtual std::vector<Jewel> LoadJewels() override {
    return lisp::ReadList<Jewel>(data_folder_ + "/jewels.lisp");
  }

  virtual std::vector<Item> LoadItems() override {
    return lisp::ReadList<Item>(data_folder_ + "/items.lisp");
  }

  virtual std::vector<Armor> LoadArmors() override {
    return lisp::ReadList<Armor>(data_folder_ + "/armors.lisp");
  }

 private:
  const std::string data_folder_;
};

// ---------- Binary Loader Core

template <typename ObjectType>
ObjectType ReadBinaryObject(std::ifstream *in);

template <typename ObjectType>
std::vector<ObjectType> ReadBinaryList(std::ifstream *in) {
  int size = 0;
  in->read(reinterpret_cast<char*>(&size), sizeof(int32_t));
  std::vector<ObjectType> result;
  result.reserve(size);
  for (int i = 0; i < size; ++i) {
    result.push_back(ReadBinaryObject<ObjectType>(in));
  }
  return result;
}

template <>
Skill ReadBinaryObject<Skill>(std::ifstream *in) {
  int points;
  in->read(reinterpret_cast<char*>(&points), sizeof(int32_t));
  return Skill(points, LanguageText());
}

template <>
SkillSystem ReadBinaryObject<SkillSystem>(std::ifstream *in) {
  int id = 0;
  in->read(reinterpret_cast<char*>(&id), sizeof(int32_t));
  SkillSystem skill_system(LanguageText(), id);
  skill_system.skills = std::move(ReadBinaryList<Skill>(in));
  return skill_system;
}

template <>
Effect ReadBinaryObject<Effect>(std::ifstream *in) {
  int skill_id = 0;
  int points = 0;
  in->read(reinterpret_cast<char*>(&skill_id), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&points), sizeof(int32_t));
  return Effect(skill_id, points);
}

template <>
Jewel ReadBinaryObject<Jewel>(std::ifstream *in) {
  Jewel jewel;
  in->read(reinterpret_cast<char*>(&jewel.external_id), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&jewel.holes), sizeof(int32_t));
  jewel.effects = std::move(ReadBinaryList<Effect>(in));
  return jewel;
}

template <>
Item ReadBinaryObject<Item>(std::ifstream *in) {
  Item item;
  in->read(reinterpret_cast<char*>(&item.id), sizeof(int32_t));
  return item;
}

template <>
ArmorPart ReadBinaryObject<ArmorPart>(std::ifstream *in) {
  int buffer;
  in->read(reinterpret_cast<char*>(&buffer), sizeof(int32_t));
  switch (buffer) {
    case 1: return HEAD;
    case 2: return BODY;
    case 3: return HANDS;
    case 4: return WAIST;
    case 5: return FEET;
    case 6: return GEAR;
    case 7: return AMULET;
    default:
      Log(WARNING, L"%d is not a recognizable part.", buffer);
      // by default, treat it as GEAR.
      return GEAR;
  }
}

template <>
WeaponType ReadBinaryObject<WeaponType>(std::ifstream *in) {
  int buffer;
  in->read(reinterpret_cast<char*>(&buffer), sizeof(int32_t));
  switch (buffer) {
  case 1: return MELEE;
  case 2: return RANGE;
  default: return BOTH;
  }
}

template <>
Gender ReadBinaryObject<Gender>(std::ifstream *in) {
  int buffer;
  in->read(reinterpret_cast<char*>(&buffer), sizeof(int32_t));
  switch (buffer) {
    case 0: return BOTH_GENDER;
    case 1: return MALE;
    case 2: return FEMALE;
    default:
      Log(WARNING, L"%d is not a recognizable gender.", buffer);
      return BOTH_GENDER;
  }
}

template <>
Resistence ReadBinaryObject<Resistence>(std::ifstream *in) {
  Resistence resistence;
  in->read(reinterpret_cast<char*>(&resistence.fire), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistence.thunder), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistence.dragon), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistence.water), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistence.ice), sizeof(int32_t));
  return resistence;
}


template <>
Armor ReadBinaryObject<Armor>(std::ifstream *in) {
  Armor armor;
  in->read(reinterpret_cast<char*>(&armor.external_id), sizeof(int32_t));
  armor.part = ReadBinaryObject<ArmorPart>(in);
  in->read(reinterpret_cast<char*>(&armor.holes), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&armor.rare), sizeof(int32_t));
  armor.type = ReadBinaryObject<WeaponType>(in);
  armor.gender = ReadBinaryObject<Gender>(in);
  in->read(reinterpret_cast<char*>(&armor.min_defense), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&armor.max_defense), sizeof(int32_t));
  armor.resistence = ReadBinaryObject<Resistence>(in);
  armor.effects = std::move(ReadBinaryList<Effect>(in));
  return armor;
}

class BinaryLoaderCore : public LoaderCore {
 public:
  BinaryLoaderCore(const std::string &path_specifier) {
    std::ifstream in(path_specifier, std::ifstream::in | std::ios::binary);
    int buffer;
    in.read(reinterpret_cast<char*>(&buffer), sizeof(int32_t));
    CHECK(7 == buffer);
    skill_system_buffer_ = std::move(ReadBinaryList<SkillSystem>(&in));
    jewel_buffer_ = std::move(ReadBinaryList<Jewel>(&in));
    item_buffer_ = std::move(ReadBinaryList<Item>(&in));
    armor_buffer_ = std::move(ReadBinaryList<Armor>(&in));
    in.close();
  }

  virtual ~BinaryLoaderCore() {}

  std::vector<SkillSystem> LoadSkillSystems() override {
    std::vector<SkillSystem> result;
    result.swap(skill_system_buffer_);
    return result;
  }

  std::vector<Jewel> LoadJewels() override {
    std::vector<Jewel> result;
    result.swap(jewel_buffer_);
    return result;
  }

  std::vector<Item> LoadItems() override {
    std::vector<Item> result;
    result.swap(item_buffer_);
    return result;
  }

  std::vector<Armor> LoadArmors() override {
    std::vector<Armor> result;
    result.swap(armor_buffer_);
    return result;
  }

 private:
  std::vector<SkillSystem> skill_system_buffer_;
  std::vector<Jewel> jewel_buffer_;
  std::vector<Item> item_buffer_;
  std::vector<Armor> armor_buffer_;
};
  


// ---------- Interface: DataLoader ----------
class DataLoader {
 public:
  DataLoader() : core_(nullptr) {}
    
  void Initialize(const std::string &descriptor) {
    std::string format;
    std::string path;
    CHECK(ParseDescriptor(descriptor, &format, &path));
    if ("standard" == format) {
      core_.reset(new StandardLoaderCore(path));
    } else if ("binary" == format) {
      core_.reset(new BinaryLoaderCore(path));
    } else {
      CHECK(false);
    }
  }

  LoaderCore *const Get() {
    return core_.get();
  }

 private:
  // Separate a path descriptor into two parts, the prefix
  // (format) and the postfix (the actual path). The delimiter is
  // ':'.
  static bool ParseDescriptor(const std::string &descriptor, 
                              std::string *prefix,
                              std::string *postfix) {
    size_t colon_index = descriptor.find_first_of(':');
    if (colon_index == std::string::npos) {
      // Fallback to standard case when we cannot find format
      // specifier in descriptor.
      *prefix = "standard";
      *postfix = descriptor;
      return true;
    }
    *prefix = descriptor.substr(0, colon_index);
    *postfix = descriptor.substr(colon_index + 1);
    return true;
  }

  std::unique_ptr<LoaderCore> core_;
};
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_DATA_LOADER_
