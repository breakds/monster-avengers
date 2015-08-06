#include <ios>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "dataset.h"


namespace monster_avengers {

namespace dataset {

namespace {

template <typename ObjectType>
ObjectType ReadBinaryObject(std::ifstream *in);

template <typename ObjectType>
ObjectType ReadBinaryObject(std::ifstream *in, int *external_id);

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

template <typename ObjectType>
void ReadBinaryTable(
    std::ifstream *in, ReindexedTable<ObjectType> *table) {
  int size = 0;
  in->read(reinterpret_cast<char*>(&size), sizeof(int32_t));
  int external_id = 0;
  ObjectType buffer;
  for (int i = 0; i < size; ++i) {
    buffer = ReadBinaryObject<ObjectType>(in, &external_id);
    table->Add(buffer, external_id);
  }
}

template <>
Skill ReadBinaryObject<Skill>(std::ifstream *in) {
  int points;
  in->read(reinterpret_cast<char*>(&points), sizeof(int32_t));
  Skill skill;
  skill.points = points;
  return skill;
}

template <>
SkillSystem ReadBinaryObject<SkillSystem>(std::ifstream *in,
                                          int *external_id) {
  int id = 0;
  in->read(reinterpret_cast<char*>(&id), sizeof(int32_t));
  SkillSystem skill_system;
  skill_system.skills = std::move(ReadBinaryList<Skill>(in));
  *external_id = id;
  return skill_system;
}

template <>
Effect ReadBinaryObject<Effect>(std::ifstream *in) {
  int id = 0;
  int points = 0;
  in->read(reinterpret_cast<char*>(&id), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&points), sizeof(int32_t));
  Effect effect;
  effect.id = id;
  effect.points = points;
  return effect;
}

template <>
Jewel ReadBinaryObject<Jewel>(std::ifstream *in,
                              int *external_id) {
  Jewel jewel;
  in->read(reinterpret_cast<char*>(external_id), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&jewel.slots), sizeof(int32_t));
  jewel.effects = std::move(ReadBinaryList<Effect>(in));
  return jewel;
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
    case 1: return WEAPON_TYPE_MELEE;
    case 2: return WEAPON_TYPE_RANGE;
    default: return WEAPON_TYPE_BOTH;
  }
}

template <>
Gender ReadBinaryObject<Gender>(std::ifstream *in) {
  int buffer;
  in->read(reinterpret_cast<char*>(&buffer), sizeof(int32_t));
  switch (buffer) {
    case 1: return GENDER_MALE;
    case 2: return GENDER_FEMALE;
    default: return GENDER_BOTH;
  }
}

template <>
Resistance ReadBinaryObject<Resistance>(std::ifstream *in) {
  Resistance resistance;
  in->read(reinterpret_cast<char*>(&resistance.fire), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistance.thunder), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistance.dragon), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistance.water), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&resistance.ice), sizeof(int32_t));
  return resistance;
}


template <>
Armor ReadBinaryObject<Armor>(std::ifstream *in, int *external_id) {
  Armor armor;
  in->read(reinterpret_cast<char*>(external_id), sizeof(int32_t));
  armor.part = ReadBinaryObject<ArmorPart>(in);
  in->read(reinterpret_cast<char*>(&armor.slots), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&armor.rare), sizeof(int32_t));
  armor.weapon_type = ReadBinaryObject<WeaponType>(in);
  armor.gender = ReadBinaryObject<Gender>(in);
  in->read(reinterpret_cast<char*>(&armor.min_defense), sizeof(int32_t));
  in->read(reinterpret_cast<char*>(&armor.max_defense), sizeof(int32_t));
  armor.resistance = ReadBinaryObject<Resistance>(in);
  armor.effects = std::move(ReadBinaryList<Effect>(in));
  return armor;
}

}  // namespace

void Data::LoadBinary(const std::string &spec) {
  armors_.ResetIndexMap();
  jewels_.ResetIndexMap();
  skills_.ResetIndexMap();

  std::ifstream in(spec, std::ifstream::in | std::ios::binary);
  ReadBinaryTable<SkillSystem>(&in, &skills_);
  ReadBinaryTable<Jewel>(&in, &jewels_);
  ReadBinaryTable<Armor>(&in, &armors_);
  InternalizeEffectIds();
  AddPredefinedArmors();
}

}  // namespace dataset

}  // namespace monster_avengers




