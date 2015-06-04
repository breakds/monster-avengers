#ifndef _MONSTER_AVENGERS_ARMOR_
#define _MONSTER_AVENGERS_ARMOR_

#include <string>

#include "language_text.h"
#include "lisp/lisp_object.h"

namespace monster_avengers {
  
  enum WeaponType {
    MELEE = 0,
    RANGE,
    BOTH,
  };

  enum ArmorPart {
    HEAD = 0,
    HANDS = 1,
    WAIST = 2,
    FEET = 3,
    GEAR = 4,
    AMULET = 5,
    BODY = 6,
    PART_NUM
  };

  enum Gender {
    MALE = 0,
    FEMALE,
    BOTH_GENDER,
  };

  namespace {
    lisp::EnumConverter<WeaponType> GetWeaponType({
        {L"Blade", MELEE}, {L"Gunner", RANGE}}, BOTH);

    lisp::EnumConverter<ArmorPart> GetArmorPart({
        {L"Head", HEAD}, {L"Body", BODY}, {L"Arms", HANDS}, {L"Waist", WAIST}, {L"Legs", FEET}, {L"gear", GEAR}});
    
    lisp::EnumConverter<Gender> GetGender({
        {L"Male", MALE}, {L"Female", FEMALE}}, BOTH_GENDER);
    
  }  // namespace

  struct Resistence : public lisp::Formattable {
    int fire;
    int thunder;
    int dragon;
    int water;
    int ice;

    Resistence() : fire(0),
                   thunder(0),
                   dragon(0),
                   water(0),
                   ice(0) {}

    Resistence(const lisp::Object &object) {
      object.AssignSlotTo("FIRE", &fire);
      object.AssignSlotTo("THUNDER", &thunder);
      object.AssignSlotTo("DRAGON", &dragon);
      object.AssignSlotTo("WATER", &water);
      object.AssignSlotTo("ICE", &ice);
    }

    lisp::Object Format() const override {
      lisp::Object result = lisp::Object::Struct();
      result["fire"] = fire;
      result["thunder"] = thunder;
      result["dragon"] = dragon;
      result["water"] = water;
      result["ice"] = ice;
      return result;
    }
  };

  struct Armor {
    LanguageText name;
    ArmorPart part;
    WeaponType type;
    Gender gender;
    int rare;
    int min_defense;
    int max_defense;
    Resistence resistence;
    int holes;
    int external_id;
    std::vector<Effect> effects;
    std::vector<int> material;
    
    Armor() = default;

    static Armor Amulet(int holes, std::vector<Effect> effects) {
      Armor armor;
      armor.name.en = L"----";
      armor.name.jp = L"----";
      armor.part = AMULET;
      armor.type = BOTH;
      armor.gender = BOTH_GENDER;
      armor.rare = 10;
      armor.min_defense = 0;
      armor.max_defense = 0;
      armor.holes = holes;
      armor.effects = std::move(effects);
      armor.external_id = -1;
      return armor;
    }

    Armor(const Armor &other) {
      name.en = other.name.en;
      name.jp = other.name.jp;
      part = other.part;
      type = other.type;
      gender = other.gender;
      rare = other.rare;
      min_defense = other.min_defense;
      max_defense = other.max_defense;
      resistence = other.resistence;
      holes = other.holes;
      effects = other.effects;
      material = other.material;
      external_id = other.external_id;
    }

    Armor(const lisp::Object &object) {
      object.AssignSlotTo("NAME", &name);
      part = GetArmorPart(object, "PART");
      gender = GetGender(object, "GENDER");
      type = GetWeaponType(object, "TYPE");
      object.AssignSlotTo("SLOTS", &holes);
      object.AssignSlotTo("RARE", &rare);
      object.AssignSlotTo("MIN-DEFENSE", &min_defense);
      object.AssignSlotTo("MAX-DEFENSE", &max_defense);
      object.AssignSlotTo("RESISTENCE", &resistence);
      object.AppendSlotTo("MATERIAL", &material);
      object.AppendSlotTo("EFFECTS", &effects);
      object.AssignSlotTo("EXTERNAL-ID", &external_id, -1);
    }
    
    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Armor {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"external_id: %d\n", external_id);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"type: %ls\n", BOTH == type ? L"BOTH" 
              : (MELEE == type ? L"MELEE" : L"RANGE"));
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"defense: %d\n", max_defense);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"rare: %d\n", rare);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"holes: %d\n", holes);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"effects: [\n");
      for (const Effect &effect : effects) {
        effect.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      wprintf(L"material: [\n");
      for (const int &id : material) {
        for (int i = 0; i < indent + 4; ++i) wprintf(L" ");
        wprintf(L"%d\n", id);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }

    bool TorsoUp() const {
      return 1 == effects.size() && effects[0].skill_id == 0;
    }
  };

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ARMOR_
