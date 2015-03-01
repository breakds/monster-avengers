#ifndef _MONSTER_AVENGERS_SEARCH_UTIL_
#define _MONSTER_AVENGERS_SEARCH_UTIL_

#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include "data/data_set.h"
#include "jewels_query.h"

namespace monster_avengers {
  
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
  
  struct TreeRoot {
    int id; // OR node id
    std::vector<Signature> jewel_keys;
    int torso_multiplier;
    
    TreeRoot(int id_) : id(id_), jewel_keys(), torso_multiplier(1) {}
    TreeRoot(int id_, int multiplier) : 
      id(id_), jewel_keys(), 
      torso_multiplier(multiplier) {}
  };

  struct TempOr {
    int id;
    int points;

    TempOr(int id_, int points_) 
      : id(id_), points(points_) {}
  };

  class SkillSplitter {
  public:
    SkillSplitter(const DataSet &data,
                  const Query &query,
                  NodePool *pool,
                  int effect_id,
                  int skill_id) 
      : pool_(pool), effect_id_(effect_id) {
      armor_points_.resize(data.armors().size());
      torso_up_.resize(data.armors().size());
      int i = 0;
      for (const Armor &armor : data.armors()) {
        armor_points_[i] = 0;
        torso_up_[i] = armor.multiplied;
        for (const Effect &effect : armor.effects) {
          if (effect.skill_id == skill_id) {
            armor_points_[i] = effect.points;
            break;
          }
        }
        i++;
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
    typedef std::unordered_map<int, std::vector<int> > PointsIdListMap;
    
    int MaxArmorOr(int or_id, int multiplier) const {
      int result = -1000;
      for (int armor_id : pool_->Or(or_id).daughters) {
        int points = armor_points_[armor_id];
        if (torso_up_[armor_id] && multiplier > 1) {
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
        if (torso_up_[armor_id] && multiplier > 1) {
          points *= multiplier;
        }
        if (points > result_max) {
          result_max = points;
        }
        auto it = new_armors->find(points);
        if (new_armors->end() != it) {
          it->second.push_back(armor_id);
        } else {
          (*new_armors)[points] = {armor_id};
        }
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
        if (torso_up_[armor_id] && multiplier > 1) {
          points *= multiplier;
        }
        if (points > result_max) {
          result_max = points;
        }
        auto it = temp_map.find(points);
        if (temp_map.end() != it) {
          it->second.push_back(armor_id);
        } else {
          temp_map[points] = {armor_id};
        }
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
              auto it = new_ands->find(points);
              if (new_ands->end() != it) {
                it->second.push_back(new_and_id);
              } else {
                (*new_ands)[points] = {new_and_id};
              }
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
      if (!new_ands.empty()) {
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
    std::vector<bool> torso_up_;
    int effect_id_;
  };
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_SEARCH_UTIL_
