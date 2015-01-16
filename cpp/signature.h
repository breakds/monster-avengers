#ifndef _MONSTER_AVENGERS_SIGNATURE_
#define _MONSTER_AVENGERS_SIGNATURE_

#include "monster_hunter_data.h"
#include "query.h"

namespace monster_avengers {
  typedef uint64_t Signature;

  namespace sig {
    Signature ArmorKey(const Armor &armor, 
                       const Query &query, 
                       bool *valid) {
      *valid = false;
      
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };

      key = 0;
      
      if (armor.holes > 0) {
        bytes[armor.holes - 1] = 1;
      }
      int byte_id = 3;
      for (const Effect& effect : query.effects) {
        for (const Effect &armor_effect : armor.effects) {
          if (effect.skill_id == armor_effect.skill_id) {
            bytes[byte_id] = static_cast<char>(armor_effect.points);
            if (armor_effect.points > 0) {
              *valid = true;
            }
          }
        }
        byte_id++;
      }
      return key;
    }

    inline Signature CombineKey(Signature a, Signature b) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      union {
        Signature a_key;
        char a_bytes[sizeof(Signature)];
      };
      union {
        Signature b_key;
        char b_bytes[sizeof(Signature)];
      };
      a_key = a;
      b_key = b;
      for (int i = 0; i < sizeof(Signature); ++i) {
        bytes[i] =  a_bytes[i] + b_bytes[i];
      }
      return key;
    }

    inline std::vector<Effect> KeyEffects(Signature input_key, 
                                          const Query &query) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      key = input_key;
      int byte_id = 3;
      std::vector<Effect> effects;
      effects.reserve(query.effects.size());
      for (int i = 0; i < query.effects.size(); ++i) {
        effects.emplace_back(query.effects[i].skill_id,
                            bytes[byte_id++]);
      }
      return effects;
    }

    inline std::vector<int> KeyPointsVec(Signature input_key, 
                                         int size) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      key = input_key;
      std::vector<int> result;
      result.reserve(size);
      for (int byte_id = 3; byte_id < 3 + size; ++byte_id) {
        result.push_back(bytes[byte_id]);
      }
      return result;
    }

  }  // namespace sig
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_SIGNATURE_
