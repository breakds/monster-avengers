#ifndef _MONSTER_AVENGERS_SIGNATURE_
#define _MONSTER_AVENGERS_SIGNATURE_

#include "monster_hunter_data.h"
#include "query.h"

namespace monster_avengers {
  typedef uint64_t Signature;

  namespace sig {
    inline Signature ArmorKey(const Armor &armor, 
                              const std::vector<Effect> &effects,
                              bool *valid) {
      *valid = false;
      
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };

      key = 0;

      if (1 == armor.holes) {
        bytes[0] = 1;
      } else if (2 == armor.holes) {
        bytes[1] = 1;
      } else if (3 == armor.holes) {
        bytes[1] = 16;
      }
      
      int byte_id = 2;
      for (const Effect& effect : effects) {
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

    inline Signature JewelKey(const Jewel &jewel, 
                              const std::vector<int> &skill_ids,
                              const std::vector<Effect> &effects,
                              bool *valid) {
      *valid = false;
      
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };

      key = 0;

      for (int skill_id : skill_ids) {
        for (const Effect &jewel_effect : jewel.effects) {
          if (skill_id == jewel_effect.skill_id) {
            if (jewel_effect.points > 0) {
              *valid = true;
            }
          }
        }
      }
      
      if (!(*valid)) return 0;

      if (1 == jewel.holes) {
        bytes[0] = 1;
      } else if (2 == jewel.holes) {
        bytes[1] = 1;
      } else if (3 == jewel.holes) {
        bytes[1] = 16;
      }

      int byte_id = 2;
      for (const Effect &effect : effects) {
        for (const Effect &jewel_effect : jewel.effects) {
          if (effect.skill_id == jewel_effect.skill_id) {
            bytes[byte_id] = static_cast<char>(jewel_effect.points);
          }
        }
        byte_id++;
      }
      return key;
    }

    inline Signature InverseKey(std::vector<Effect>::const_iterator begin,
                                std::vector<Effect>::const_iterator end) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      
      key = 0;
      int byte_id = 2;
      for (auto it = begin; it != end; ++it) {
        bytes[byte_id++] = -it->points;
      }
      return key;
    }

    inline Signature CombineKey(Signature a, Signature b) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      union {
        Signature key_a;
        char bytes_a[sizeof(Signature)];
      };
      union {
        Signature key_b;
        char bytes_b[sizeof(Signature)];
      };
      key_a = a;
      key_b = b;
      for (int i = 0; i < sizeof(Signature); ++i) {
        bytes[i] =  bytes_a[i] + bytes_b[i];
      }
      return key;
    }

    inline Signature CombineKeyPoints(Signature a, Signature b) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      union {
        Signature key_a;
        char bytes_a[sizeof(Signature)];
      };
      union {
        Signature key_b;
        char bytes_b[sizeof(Signature)];
      };
      key = 0;
      key_a = a;
      key_b = b;
      
      for (int i = 2; i < sizeof(Signature); ++i) {
        bytes[i] = bytes_a[i] + bytes_b[i];
      }
      
      return key;
    }

    inline Signature AddPoints(Signature input_key, 
                               int effect_id, int points) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      
      key = input_key;
      bytes[2 + effect_id] += points;
      
      return key;
    }

    inline int GetPoints(Signature input_key, int effect_id) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      
      key = input_key;
      return bytes[2 + effect_id];
    }

    inline std::vector<Effect> KeyEffects(Signature input_key, 
                                          const std::vector<Effect> &required) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      key = input_key;
      int byte_id = 2;
      std::vector<Effect> result;
      result.reserve(required.size());
      for (int i = 0; i < required.size(); ++i) {
        result.emplace_back(required[i].skill_id,
                            bytes[byte_id++]);
      }
      return result;
    }

    inline std::vector<Effect> KeyEffects(Signature input_key, 
                                          const Query &query) {
      return KeyEffects(input_key, query.effects);
    }

    inline void KeyHoles(Signature input_key, 
                         int *one, int *two, int *three) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      key = input_key;
      *one = bytes[0];
      *two = bytes[1] & 15;
      *three = bytes[1] >> 4;
    }

    inline Signature HolesToKey(int one, int two, int three) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      key = 0;
      bytes[0] = one;
      bytes[1] = two;
      bytes[1] |= (three << 4);
      return key;
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
      for (int byte_id = 2; byte_id < 2 + size; ++byte_id) {
        result.push_back(bytes[byte_id]);
      }
      return result;
    }

    void ExplainSignature(Signature key,
                          const std::vector<Effect> &required) {
      int i(0), j(0), k(0);
      KeyHoles(key, &i ,&j, &k);
      std::vector<Effect> effects(std::move(KeyEffects(key, required)));
      wprintf(L"{\n");
      if (0 < i) wprintf(L"  %d x [O]\n", i);
      if (0 < j) wprintf(L"  %d x [OO]\n", j);
      if (0 < k) wprintf(L"  %d x [OOO]\n", k);
      for (const Effect &effect : effects) {
        wprintf(L"  %d: %d\n", effect.skill_id, effect.points);
      }
      wprintf(L"}\n");
    }

    inline bool Satisfy(Signature test, Signature inverse_target) {
      return !(CombineKey(test, inverse_target) & 0x8080808080800000);
    }

  }  // namespace sig
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_SIGNATURE_
