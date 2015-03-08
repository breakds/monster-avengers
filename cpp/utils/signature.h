#ifndef _MONSTER_AVENGERS_SIGNATURE_
#define _MONSTER_AVENGERS_SIGNATURE_

#include <cstring>
#include <functional>
#include "data/data_set.h"
#include "utils/query.h"

namespace monster_avengers {

  // Signature is a 16 bytes (128 bit) unsigned integer, internally
  // represented as a length-16 char array. Where the encoding
  // follows:
  // 
  // byte 0: number of 1-slots [8]
  //
  // byte 1: number of 3-slots [4] | number of 2 slots [4]
  // 
  // byte 2: sign [1] | number of body 3-slots [1] | number of body 2-slots [1] |
  //         number of body 1-slots [2] | torso multiplier [3]
  // 
  // byte 3+: number of points for corresponding skill system [8]
  //
  // Notes: [x] stands for x bits in the byte.
  struct Signature {
    static constexpr int EFFECTS_BEGIN = 3;
    char bytes[16];

    inline Signature() {
      memset(bytes, 0, sizeof(Signature));
    }

    inline Signature(const Armor &armor, const std::vector<Effect> &effects) 
      : Signature() {
      if (armor.TorsoUp()) {
        // Torso up armors are not allowedto have holes and effects.
        bytes[2] = 1;
        return;
      } else if (BODY == armor.part) {
        bytes[2] = 1;
      } else {
        bytes[2] = 0;
      }
      
      if (1 == armor.holes) {
        bytes[0] = 1;
      } else if (2 == armor.holes) {
        bytes[1] = 1;
      } else if (3 == armor.holes) {
        bytes[1] = 16;
      }
      
      int byte_id = EFFECTS_BEGIN;
      for (const Effect& effect : effects) {
        for (const Effect &armor_effect : armor.effects) {
          if (effect.skill_id == armor_effect.skill_id) {
            bytes[byte_id] = static_cast<char>(armor_effect.points);
          }
        }
        byte_id++;
      }
    }

    inline Signature(const Jewel &jewel, 
              const std::vector<int> &skill_ids,
              const std::vector<Effect> &effects,
              bool *valid)
      : Signature() {
      *valid = false;

      for (int skill_id : skill_ids) {
        for (const Effect &jewel_effect : jewel.effects) {
          if (skill_id == jewel_effect.skill_id) {
            if (jewel_effect.points > 0) {
              *valid = true;
            }
          }
        }
      }
      
      if (!(*valid)) return;

      if (1 == jewel.holes) {
        bytes[0] = 1;
      } else if (2 == jewel.holes) {
        bytes[1] = 1;
      } else if (3 == jewel.holes) {
        bytes[1] = 16;
      }

      int byte_id = EFFECTS_BEGIN;
      for (const Effect &effect : effects) {
        for (const Effect &jewel_effect : jewel.effects) {
          if (effect.skill_id == jewel_effect.skill_id) {
            bytes[byte_id] = static_cast<char>(jewel_effect.points);
          }
        }
        byte_id++;
      }
    }

    // ---------- Signature Methods ----------

    void ShowMetaInfo() const {
      int one = bytes[0];
      int two = bytes[1] & 15;
      int three = bytes[1] >> 4;
      wprintf(L"(%d %d %d) + %d, %d\n",
              one, two, three, BodyHoleSum(), multiplier());
    }
    
    inline void BodyRefactor(int multiplier) {
      if (multiplier > 1) {
        // Transfer Holes
        if (0 != bytes[0]) {
          bytes[2] |= (0x08 * bytes[0]);
        } 
        
        if (0 != (bytes[1] & 0x0F)) {
          bytes[2] |= 0x20;
        } 
        
        if (0 != (bytes[1] & 0xF0)) {
          bytes[2] |= 0x40;
        }

        bytes[0] = 0;
        bytes[1] = 0;
        
        for (int i = EFFECTS_BEGIN; i < sizeof(Signature); ++i) {
          bytes[i] *= multiplier;
        }
      }
    }

    inline int BodyHoleSum() const {
      int result = 0;
      if (0 != (bytes[2] & 0x40)) {
        return 3;
      } else if (0 != (bytes[2] & 0x20)) {
        result = 2;
      }
      result += (bytes[2] & 0x18) >> 3;
      return result;
    }

    void BodyHoles(int *i, int *j, int *k) const {
      *k = (0 != (bytes[2] & 0x40)) ? 1 : 0;
      *j = (0 != (bytes[2] & 0x20)) ? 1 : 0;
      *i = (bytes[2] & 0x18) >> 3;
    }


    int multiplier() const {
      return static_cast<int>(bytes[2] & 0x07);
    }

    bool IsZero() const {
      for (int i = 0; i < sizeof(Signature); ++i) {
        if (bytes[i] != 0) return false;
      }
      return true;
    }

    bool operator==(const Signature &other) const {
      for (int i = 0; i < sizeof(Signature); ++i) {
        if (bytes[i] != other.bytes[i]) return false;
      }
      return true;
    }

    inline void operator*=(int multiplier) {
      for (int i = EFFECTS_BEGIN; i < sizeof(Signature); ++i) {
        bytes[i] *= multiplier;
      }
    }

    inline void operator+=(const Signature &other) {
      for (int i = 0; i < sizeof(Signature); ++i) {
        bytes[i] += other.bytes[i];
      }
    }
  };

  inline Signature operator+(const Signature &a, const Signature &b) {
    Signature key;
    for (int i = 0; i < sizeof(Signature); ++i) {
      key.bytes[i] =  a.bytes[i] + b.bytes[i];
    }
    return key;
  }

  inline Signature operator|(const Signature &a, const Signature &b) {
    Signature key;
    for (int i = Signature::EFFECTS_BEGIN; i < sizeof(Signature); ++i) {
      key.bytes[i] = a.bytes[i] + b.bytes[i];
    }
    return key;
  }


  
  namespace sig {
    constexpr int EFFECTS_BEGIN = Signature::EFFECTS_BEGIN;

    inline Signature InverseKey(std::vector<Effect>::const_iterator begin,
                                std::vector<Effect>::const_iterator end) {
      
      Signature key;
      char *bytes = reinterpret_cast<char*>(&key);
      
      int byte_id = EFFECTS_BEGIN;
      for (auto it = begin; it != end; ++it) {
        bytes[byte_id++] = -it->points;
      }
      return key;
    }

    inline Signature InverseKey(Signature input_key) {
      Signature key = input_key;
      key.bytes[0] = 0;
      key.bytes[1] = 0;
      char *bytes = reinterpret_cast<char*>(&key);
      for (int i = EFFECTS_BEGIN; i < sizeof(Signature); ++i) {
        bytes[i] = -bytes[i];
      }
      return key;
    }

    inline Signature AddPoints(Signature input_key, 
                               int effect_id, int points) {
      Signature key = input_key;
      char *bytes = reinterpret_cast<char*>(&key);
      bytes[EFFECTS_BEGIN + effect_id] += points;
      return key;
    }
    
    inline int GetPoints(Signature key, int effect_id) {
      char *bytes = reinterpret_cast<char*>(&key);
      return bytes[EFFECTS_BEGIN + effect_id];
    }

    inline std::vector<Effect> KeyEffects(Signature key, 
                                          const std::vector<Effect> &required) {
      char *bytes = reinterpret_cast<char*>(&key);
      int byte_id = EFFECTS_BEGIN;
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
      Signature key;
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
      for (int byte_id = EFFECTS_BEGIN; 
           byte_id < EFFECTS_BEGIN + size; 
           ++byte_id) {
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

    inline bool Satisfy(const Signature &test, 
                        const Signature &inverse_target) {
      for (int i = EFFECTS_BEGIN; i < sizeof(Signature); ++i) {
        if (test.bytes[i] + inverse_target.bytes[i] < 0) return false;
      }
      return true;
    }

  }  // namespace sig
  
}  // namespace monster_avengers

namespace std {
  // Jenkins Hash Function
  // The code is a slightly modified version from the wikipedia page.
  template <>
  struct hash<monster_avengers::Signature> {
    uint32_t operator()(const monster_avengers::Signature &input) const {
      uint32_t hash = 0;
      for (int i = 0; i < sizeof(monster_avengers::Signature); ++i) {
        hash += input.bytes[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
      }
      hash += (hash << 3);
      hash ^= (hash >> 11);
      hash += (hash << 15);
      return hash;
    }
  };
}  // namespace std
  


#endif  // _MONSTER_AVENGERS_SIGNATURE_
