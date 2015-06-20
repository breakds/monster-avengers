#ifndef _MONSTER_AVENGERS_FILTER_
#define _MONSTER_AVENGERS_FILTER_

#include "dataset/core/armor.h"
#include "dataset/core/jewel.h"

using namespace monster_avengers::dataset;

namespace monster_avengers {

struct ArmorFilter {
  int min_rare;
  int max_rare;
  WeaponType weapon_type;
  int weapon_slots;
  Gender gender;
  // Blacklist has higher priority.
  std::unordered_set<int> whitelist;
  std::unordered_set<int> blacklist;

  ArmorFilter() = default;
    
  bool Validate(const Armor &armor, int id) const {
    // Armor ID Blacklist/Whitelist.
    if (0 != blacklist.count(id)) return false;
    if (armor.part != GEAR && armor.part != AMULET &&
        0 < whitelist.size() &&
        0 == whitelist.count(id)) {
      return false;
    }
    
    // Weapon Type should match.
    if (armor.weapon_type != weapon_type &&
        WEAPON_TYPE_BOTH != armor.weapon_type) return false;

    // Gender constraint.
    if (armor.gender != gender && GENDER_BOTH != armor.gender) return false;
 
    // Rarity Constraint should be enforced on non GEAR/AMULET.
    if (GEAR != armor.part && AMULET != armor.part) {
      if (armor.rare < min_rare || armor.rare > max_rare) {
        return false;
      }
    }
    
    // Weapon slots contraint.
    if (GEAR == armor.part && armor.slots != weapon_slots) return false;

    return true;
  }
};

struct JewelFilter {
  std::unordered_set<int> blacklist;

  bool Validate(int id) const {
    return 0 == blacklist.count(id);
  }
};
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_FILTER_
