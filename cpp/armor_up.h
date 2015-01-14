#ifndef _MONSTER_AVENGERS_ARMOR_UP_
#define _MONSTER_AVENGERS_ARMOR_UP_

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "query.h"

namespace monster_avengers {

  typedef uint64_t Signature;

  enum ORTag {
    ANDS = 0,
    ARMORS
  };
    
  struct OR {
    Signature key;
    ORTag tag;
    std::vector<int> daughters;
      
    OR(Signature key_, ORTag tag_, 
       std::vector<int> *daughters_) :
      key(key_),
      tag(tag_) {
      daughters.swap(*daughters_);
    }
  };

  struct AND {
    Signature key;
    int left;
    int right;
    
    AND(int left_, int right_) 
      : left(left_), right(right_) {}
  };

  class NodePool {
  public:
    NodePool() : or_pool_(), and_pool_() {}
    
    // Returns the index of the newly created OR node.
    template <ORTag Tag>
    int MakeOR(Signature key, std::vector<int> *daughters) {
      or_pool_.emplace_back(key, Tag, daughters);
      return or_pool_.size() - 1;
    }

    int MakeAnd(int left, int right) {
      and_pool_.emplace_back(left, right);
      return and_pool_.size() - 1;
    }
    
    inline const OR &Or(int id) {
      return or_pool_[id];
    }

    inline const AND &And(int id) {
      return and_pool_[id];
    }

    inline size_t OrSize() {
      return or_pool_.size();
    }

    inline size_t AndSize() {
      return and_pool_.size();
    }
  private:
    std::vector<OR> or_pool_;
    std::vector<AND> and_pool_;
  };

  class ArmorUp {
  public:
    ArmorUp(const std::string &data_folder) 
      : data_(data_folder), pool_() {}
    

    std::vector<int> SearchFoundation(const Query &query) {
      std::vector<int> previous;
      std::vector<int> current;
      for (ArmorPart part = HEAD; part < NECK; ++part) {
        previous.swap(current);
        current = std::move(ClassifyArmors(part, query));
        if (HEAD != part) {
          current = std::move(MergeForests(current, previous));
        }
      }
      return current;
    }

    void Summarize() {
      data_.Summarize();
      Log(INFO, L"OR Nodes: %lld\n", pool_.OrSize());
      Log(INFO, L"AND Nodes: %lld\n", pool_.AndSize());
    }

  private:
    static Signature GetKey(const Armor &armor, 
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

    inline static Signature CombineKey(Signature a, Signature b) {
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
    
    // Returns a vector of newly created or nodes' indices.
    std::vector<int> ClassifyArmors(ArmorPart part, 
                                    const Query &query) {
      std::unordered_map<Signature, std::vector<int> > armor_map;
      bool valid = false;
      for (int id : data_.ArmorIds(part)) {
        const Armor &armor = data_.armor(id);
        if (armor.type == query.weapon_type || BOTH == armor.type) {
          Signature key = GetKey(armor, query, &valid);
          if (valid) {
            armor.DebugPrint();
            auto it = armor_map.find(key);
            if (armor_map.end() == it) {
              armor_map[key] = {id};
            } else {
              it->second.push_back(id);
            }
          }
        }
      }

      std::vector<int> forest;
      forest.reserve(armor_map.size());
      for (auto &item : armor_map) {
        forest.push_back(pool_.MakeOR<ARMORS>(item.first, 
                                              &item.second));
      }
      return forest;
    }

    std::vector<int> MergeForests(std::vector<int> left_ors, 
                                  std::vector<int> right_ors) {
      std::unordered_map<Signature, std::vector<int> > and_map;
      for (int i : left_ors) {
        const OR &left = pool_.Or(i);
        for (int j : right_ors) {
          const OR &right = pool_.Or(j);
          Signature key = CombineKey(left.key, right.key);
          int id = pool_.MakeAnd(i, j);
          auto it = and_map.find(key);
          if (and_map.end() == it) {
            and_map[key] = {id};
          } else {
            it->second.push_back(id);
          }
        }
      }

      std::vector<int> forest;
      forest.reserve(and_map.size());
      for (auto &item : and_map) {
        forest.push_back(pool_.MakeOR<ANDS>(item.first,
                                            &item.second));
      }
      return forest;
    }

    DataSet data_;
    NodePool pool_;
  };
}

#endif  // _MONSTER_AVENGERS_ARMOR_UP_

