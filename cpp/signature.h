#ifndef _MONSTER_AVENGERS_SIGNATURE_
#define _MONSTER_AVENGERS_SIGNATURE_

#include "monster_hunter_data.h"
#include "query.h"

namespace monster_avengers {
  typedef uint64_t Signature;

  class HoleCoder {
  public:

    HoleCoder() {
      char i = 0;
      char j = 0;
      char k = 0;
      char code = 0;
      do {
        int semi_key = i | (j << 3) | (k << 6);
        ones_[code] = i;
        twos_[code] = j;
        threes_[code] = k;
        code_[semi_key] = code++;
        if (i + j + k < 7) {
          ++i;
        } else {
          i = 0;
          if (j + k < 7) {
            ++j;
          } else {
            j = 0;
            ++k;
          }
        }
      } while (k < 8);
    }

    inline char Encode(int holes) {
      return (holes > 0) ? code_[1 << (holes - 1) * 3] : 0;
    }

    inline char Encode(int one, 
                       int two, 
                       int three) {
      int semi_key = one | (two << 3) | (three << 6);
      return code_[semi_key];
    }

    inline void Decode(char key, 
                       int *one, 
                       int *two, 
                       int *three) {
      *one = ones_[key];
      *two = twos_[key];
      *three = threes_[key];
    }

    inline void DecodeTo(char key, 
                         int *one, 
                         int *two, 
                         int *three) {
      *one += ones_[key];
      *two += twos_[key];
      *three += threes_[key];
    }

    inline char Combine(char key_a,
                        char key_b) {
      int one(0), two(0), three(0);
      Decode(key_a, &one, &two, &three);
      DecodeTo(key_b, &one, &two, &three);
      return Encode(one, two, three);
    }
    
  private:
    char ones_[256];
    char twos_[256];
    char threes_[256];
    char code_[512];
  };

  HoleCoder hole_coder;

  namespace sig {
    inline Signature ArmorKey(const Armor &armor, 
                              const Query &query, 
                              bool *valid) {
      *valid = false;
      
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };

      key = 0;
      bytes[0] = hole_coder.Encode(armor.holes);
      int byte_id = 1;
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

    inline Signature JewelKey(const Jewel &jewel, 
                              std::vector<Effect>::const_iterator effects_begin,
                              std::vector<Effect>::const_iterator effects_end,
                              bool *valid) {
      *valid = false;
      
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };

      key = 0;
      bytes[0] = hole_coder.Encode(jewel.holes);
      int byte_id = 1;
      for (auto it = effects_begin; it != effects_end; ++it) {
        for (const Effect &jewel_effect : jewel.effects) {
          if (it->skill_id == jewel_effect.skill_id) {
            bytes[byte_id] = static_cast<char>(jewel_effect.points);
            if (jewel_effect.points > 0) {
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
        Signature key_a;
        char bytes_a[sizeof(Signature)];
      };
      union {
        Signature key_b;
        char bytes_b[sizeof(Signature)];
      };
      key_a = a;
      key_b = b;
      bytes[0] = hole_coder.Combine(bytes_a[0], bytes_b[0]);
      for (int i = 1; i < sizeof(Signature); ++i) {
        bytes[i] =  bytes_a[i] + bytes_b[i];
      }
      return key;
    }

    inline std::vector<Effect> KeyEffects(Signature input_key, 
                                          const std::vector<Effect> &required) {
      union {
        Signature key;
        char bytes[sizeof(Signature)];
      };
      key = input_key;
      int byte_id = 1;
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
      hole_coder.Decode(bytes[0], one, two, three);
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
      for (int byte_id = 1; byte_id < 1 + size; ++byte_id) {
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

  }  // namespace sig
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_SIGNATURE_
