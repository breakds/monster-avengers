#ifndef _MONSTER_AVENGERS_SEARCH_UTIL_
#define _MONSTER_AVENGERS_SEARCH_UTIL_

#include <algorithm>
#include <vector>
#include <array>
#include "monster_hunter_data.h"

namespace monster_avengers {

  typedef std::array<int, PART_NUM> ArmorSet;
  
  void OutputArmorSet(const DataSet &data, 
                      const ArmorSet &armor_set) {
    wprintf(L"---------- Armor Set ----------\n");
    std::vector<Effect> effects;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor armor = data.armor(armor_set[i]);
      wprintf(L"[");
      for (int j = 0; j < 3; ++j) {
        if (j < armor.holes) {
          wprintf(L"O");
        } else {
          wprintf(L"-");
        }
      }
      wprintf(L"] %ls\n", armor.name.c_str());
      for (const Effect &effect : armor.effects) {
        auto it = std::find_if(effects.begin(), effects.end(),
                               [&effect](const Effect& x) {
                                 return x.skill_id == effect.skill_id;
                               });
        if (effects.end() == it) {
          effects.push_back(effect);
        } else {
          it->points += effect.points;
        }
      }
    }
    for (const Effect &effect : effects) {
      wprintf(L"%ls(%d)  ", 
              data.skill_system(effect.skill_id).name.c_str(),
              effect.points);
    }
    wprintf(L"\n\n");
  }
  
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
    
    inline const OR &Or(int id) const {
      return or_pool_[id];
    }

    inline const AND &And(int id) const {
      return and_pool_[id];
    }

    inline const OR &AndLeftOr(int and_id) const {
      return or_pool_[and_pool_[and_id].left];
    }

    inline const AND &OrAnd(int or_id, int and_id) const {
      return and_pool_[or_pool_[or_id].daughters[and_id]];
    }

    inline size_t OrSize() const {
      return or_pool_.size();
    }

    inline size_t AndSize() const {
      return and_pool_.size();
    }
  private:
    std::vector<OR> or_pool_;
    std::vector<AND> and_pool_;
  };
  
}

#endif  // _MONSTER_AVENGERS_SEARCH_UTIL_
