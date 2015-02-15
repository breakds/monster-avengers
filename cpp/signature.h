#ifndef _MONSTER_AVENGERS_SIGNATURE_
#define _MONSTER_AVENGERS_SIGNATURE_

#include "monster_hunter_data.h"
#include "query.h"

namespace monster_avengers {
  typedef uint64_t Signature;

  namespace sig {
    inline Signature ArmorKey(const Armor &armor, 
                              const std::vector<Effect> &effects) {
      Signature key = 0;
      char *bytes = reinterpret_cast<char *>(&key);
      
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
          }
        }
        byte_id++;
      }
      return key;
    }

    // Construct a signature with hole alignment (i, j, k) and the
    // given effect points. This is mainly for debug use.
    inline Signature ConstructKey(int i, int j, int k,
                                  std::vector<int> points) {
      Signature key = 0;
      char *bytes = reinterpret_cast<char*>(&key);
      bytes[0] = i;
      bytes[1] = j;
      bytes[1] |= (k << 4);
      int byte_id = 2;
      for (int value : points) {
        bytes[byte_id++] = value;
      }
      return key;
    }
    
    inline Signature JewelKey(const Jewel &jewel, 
                              const std::vector<int> &skill_ids,
                              const std::vector<Effect> &effects,
                              bool *valid) {
      *valid = false;

      Signature key = 0;
      char *bytes = reinterpret_cast<char*>(&key);
      
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
      
      Signature key = 0;
      char *bytes = reinterpret_cast<char*>(&key);
      
      int byte_id = 2;
      for (auto it = begin; it != end; ++it) {
        bytes[byte_id++] = -it->points;
      }
      return key;
    }

    inline Signature InverseKey(Signature input_key) {
      Signature key = input_key & 0xffffffffffff0000;
      char *bytes = reinterpret_cast<char*>(&key);
      for (int i = 2; i < sizeof(Signature); ++i) {
        bytes[i] = -bytes[i];
      }
      return key;
    }

    inline void KeyMultiplication(Signature *a, int multiplier) {
      char *bytes = reinterpret_cast<char*>(a);
      for (int i = 2; i < sizeof(Signature); ++i) {
        bytes[i] *= multiplier;
      }
    }

    inline Signature CombineKey(Signature a, Signature b) {
      Signature key = 0;
      char *bytes = reinterpret_cast<char*>(&key);
      char *bytes_a = reinterpret_cast<char*>(&a);
      char *bytes_b = reinterpret_cast<char*>(&b);
      
      for (int i = 0; i < sizeof(Signature); ++i) {
        bytes[i] =  bytes_a[i] + bytes_b[i];
      }
      return key;
    }

    inline Signature CombineKeyPoints(Signature a, Signature b) {
      Signature key = 0;
      char *bytes = reinterpret_cast<char*>(&key);
      char *bytes_a = reinterpret_cast<char*>(&a);
      char *bytes_b = reinterpret_cast<char*>(&b);
      
      for (int i = 2; i < sizeof(Signature); ++i) {
        bytes[i] = bytes_a[i] + bytes_b[i];
      }
      
      return key;
    }

    inline Signature AddPoints(Signature input_key, 
                               int effect_id, int points) {
      Signature key = input_key;
      char *bytes = reinterpret_cast<char*>(&key);
      bytes[2 + effect_id] += points;
      return key;
    }

    inline int GetPoints(Signature key, int effect_id) {
      char *bytes = reinterpret_cast<char*>(&key);
      return bytes[2 + effect_id];
    }

    inline std::vector<Effect> KeyEffects(Signature key, 
                                          const std::vector<Effect> &required) {
      char *bytes = reinterpret_cast<char*>(&key);
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

    inline void KeyHoles(Signature key, 
                         int *one, int *two, int *three) {
      char *bytes = reinterpret_cast<char*>(&key);
      *one = bytes[0];
      *two = bytes[1] & 15;
      *three = bytes[1] >> 4;
    }

    inline Signature HolesToKey(int one, int two, int three) {
      Signature key = 0;
      char *bytes = reinterpret_cast<char*>(&key);
      bytes[0] = one;
      bytes[1] = two;
      bytes[1] |= (three << 4);
      return key;
    }

    inline std::vector<int> KeyPointsVec(Signature key, 
                                         int size) {
      char *bytes = reinterpret_cast<char*>(&key);
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
