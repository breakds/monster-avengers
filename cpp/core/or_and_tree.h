#ifndef _MONSTER_AVENGERS_SEARCH_UTIL_
#define _MONSTER_AVENGERS_SEARCH_UTIL_

#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include "dataset/dataset.h"
#include "utils/jewels_query.h"
#include "utils/tiny_map.h"

namespace monster_avengers {

using dataset::Arsenal;
using dataset::Data;
  
enum ORTag {
  ANDS = 0,
  ARMORS
};

struct OR {
  Signature key;
  ORTag tag;
  std::vector<int> daughters;

  OR() = default;
      
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

  AND() = default;
    
  AND(int left_, int right_) 
      : left(left_), right(right_) {}
};

class NodePool {
 public:
  struct Snapshot {
    Snapshot(size_t or_size_, size_t and_size_)
        : or_size(or_size_), and_size(and_size_) {}
    size_t or_size;
    size_t and_size;
  };
    
  NodePool() : or_pool_(), and_pool_(), snapshots_() {}
    
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

  inline void PushSnapshot() {
    snapshots_.emplace_back(or_pool_.size(), and_pool_.size());
  }

  inline void PopSnapshot() {
    or_pool_.resize(snapshots_.back().or_size);
    and_pool_.resize(snapshots_.back().and_size);
    snapshots_.pop_back();
  }
    
  inline void RestoreSnapshot() {
    or_pool_.resize(snapshots_.back().or_size);
    and_pool_.resize(snapshots_.back().and_size);
  }

  inline void Reset() {
    or_pool_.clear();
    and_pool_.clear();
    snapshots_.clear();
  }

 private:
  std::vector<OR> or_pool_;
  std::vector<AND> and_pool_;
  std::vector<Snapshot> snapshots_;
};
  
struct TreeRoot {
  int id; // OR node id
  std::vector<Signature> jewel_keys;
  int torso_multiplier;
    
  TreeRoot(int id_) : id(id_), jewel_keys(), torso_multiplier(1) {}
  TreeRoot(int id_, const OR &node) : 
      id(id_), jewel_keys(), 
      torso_multiplier(node.key.multiplier()) {}
};

struct TempOr {
  int id;
  int points;

  TempOr(int id_, int points_) 
      : id(id_), points(points_) {}
};

class SkillSplitter {
 public:
  SkillSplitter(const Arsenal &arsenal,
                NodePool *pool,
                int effect_id,
                int skill_id) : pool_(pool), effect_id_(effect_id) {
    armor_points_.resize(arsenal.size());
    is_body_.resize(arsenal.size());
    int i = 0;
    for (int i = 0; i < arsenal.size(); ++i) {
      const Armor &armor = arsenal[i];
      armor_points_[i] = 0;
      is_body_[i] = armor.part == BODY;
      for (const Effect &effect : armor.effects) {
        if (effect.id == skill_id) {
          armor_points_[i] = effect.points;
          break;
        }
      }
    }
  }

  inline int Max(const TreeRoot &root) const {
    return MaxOr(root.id, root.torso_multiplier);
  }

  inline std::vector<int> Split(const TreeRoot &root, int sub_min) {
    std::vector<TempOr> temp_ors;
    std::vector<int> result;
    SplitOr(root.id, sub_min, &temp_ors, 
            root.torso_multiplier);
    for (TempOr &item : temp_ors) {
      result.push_back(item.id);
    }
    return result;
  }
    
 private:
  typedef CompositeListMap PointsIdListMap;
    
  int MaxArmorOr(int or_id, int multiplier) const {
    int result = -1000;
    for (int armor_id : pool_->Or(or_id).daughters) {
      int points = armor_points_[armor_id];
      if (is_body_[armor_id] && multiplier > 1) {
        points *= multiplier;
      }
      if (points > result) {
        result = points;
      }
    }
    return result;
  }

  int MaxAnd(int and_id, int multiplier) const {
    const AND &node = pool_->And(and_id);
    const OR &right_node = pool_->Or(node.right);
    return MaxArmorOr(node.left, multiplier) + 
        (ANDS == right_node.tag ? MaxOr(node.right, multiplier) : 
         MaxArmorOr(node.right, multiplier));
  }
    
  int MaxOr(int or_id, int multiplier) const {
    int result = -1000;
    for (int and_id : pool_->Or(or_id).daughters) {
      int tmp = MaxAnd(and_id, multiplier);
      if (tmp > result) {
        result = tmp;
      }
    }
    return result;
  }

  int SplitArmorOr(int or_id, PointsIdListMap *new_armors, 
                   int multiplier) {
    int result_max = -1000;
    for (int armor_id : pool_->Or(or_id).daughters) {
      int points = armor_points_[armor_id];
      if (is_body_[armor_id] && multiplier > 1) {
        points *= multiplier;
      }
      if (points > result_max) {
        result_max = points;
      }

      new_armors->Push(points, armor_id);
    }
    return result_max;
  }

  int SplitArmorOr(int or_id, std::vector<TempOr> *new_armors, 
                   int sub_min, int multiplier) {
    PointsIdListMap temp_map;
    int result_max = -1000;
    const OR &node = pool_->Or(or_id);
    for (int armor_id : node.daughters) {
      int points = armor_points_[armor_id];
      if (is_body_[armor_id] && multiplier > 1) {
        points *= multiplier;
      }
      if (points > result_max) {
        result_max = points;
      }

      temp_map.Push(points, armor_id);
    }

    // node may be invalid below due to MakeOR<ARMORS>().
    Signature key = node.key;
      
    for (auto &item : temp_map) {
      int new_or_id = pool_->MakeOR<ARMORS>(sig::AddPoints(key,
                                                           effect_id_,
                                                           item.first),
                                            &item.second);
      new_armors->emplace_back(new_or_id, item.first);
    }
      
    return result_max;
  }


  void SplitAnd(int and_id, int sub_min, 
                PointsIdListMap *new_ands, int multiplier) {
    PointsIdListMap split_armors;
    const AND &node = pool_->And(and_id);
    int left_max = SplitArmorOr(node.left, &split_armors, multiplier);
    std::vector<TempOr> split_right;
    const OR &right_node = pool_->Or(node.right);
    int right_max = (ANDS == right_node.tag) ?
        SplitOr(node.right, sub_min - left_max,
                &split_right, multiplier) :
        SplitArmorOr(node.right, &split_right, 
                     sub_min - left_max, multiplier);

    // Note(breakds), node may already have been invalid as the
    // above code may trigger reallocation of vector in pool_.
      
    Signature left_key = pool_->Or(pool_->And(and_id).left).key;
    for (auto &left_item : split_armors) {
      if (right_max + left_item.first >= sub_min) {
        int left_or_id = 
            pool_->MakeOR<ARMORS>(sig::AddPoints(left_key,
                                                 effect_id_,
                                                 left_item.first),
                                  &left_item.second);
        for (auto &right_item : split_right) {
          int points = left_item.first + right_item.points;
          if (points >= sub_min) {
            int new_and_id = pool_->MakeAnd(left_or_id,
                                            right_item.id);
            new_ands->Push(points, new_and_id);
          }
        }
      }
    }
  }

  int SplitOr(int or_id, int sub_min, 
              std::vector<TempOr> *result, int multiplier) {
    PointsIdListMap new_ands;
    for (int and_id : pool_->Or(or_id).daughters) {
      SplitAnd(and_id, sub_min, &new_ands, multiplier);
    }

    int result_max = -1000;
    if (0 < new_ands.size()) {
      Signature key = pool_->Or(or_id).key;
      for (auto &item : new_ands) {
        result->emplace_back(pool_->MakeOR<ANDS>(sig::AddPoints(key,
                                                                effect_id_,
                                                                item.first),
                                                 &item.second),
                             item.first);
        if (item.first > result_max) {
          result_max = item.first;
        }
      }
    }
    return result_max;
  }

    
    
  NodePool *pool_;
  std::vector<int> armor_points_;
  std::vector<bool> is_body_;
  int effect_id_;
};
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_SEARCH_UTIL_
