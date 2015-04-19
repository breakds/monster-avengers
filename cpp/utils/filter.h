#ifndef _MONSTER_AVENGERS_FILTER_
#define _MONSTER_AVENGERS_FILTER_

#include "data/armor.h"

namespace monster_avengers {

  struct ArmorFilter {
    int min_rare;
    int max_rare;
    WeaponType weapon_type;
    int weapon_holes;
    std::unordered_set<int> blacklist;

    bool Validate(const DataSet &data, int id) const {
      const Armor &armor = data.armor(id);

      // Weapon Type should match.
      if (armor.type != weapon_type && BOTH != armor.type) return false;

      // Rarity Constraint should be enforced on non GEAR/AMULET.
      if (GEAR != armor.part && AMULET != armor.part) {
	if (armor.rare < min_rare || armor.rare > max_rare) {
	  return false;
	}
      }

      // Weapon holes contraint.
      if (GEAR == armor.part && armor.holes != weapon_holes) return false;

      // Armor ID Blacklist.
      if (armor.external_id == -1) {
	if (0 != blacklist.count(id)) return false;
      } else {
	if (0 != blacklist.count(armor.external_id)) return false;
      }

      return true;
    }
  };

  struct JewelFilter {
    std::unordered_set<int> blacklist;

    bool Validate(const DataSet &data, int id) const {
      const Jewel &jewel = data.jewel(id);
      if (jewel.external_id == -1) {
	return 0 == blacklist.count(id);
      }
      return 0 == blacklist.count(jewel.external_id);
    }
  };
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_FILTER_
