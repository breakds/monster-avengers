#ifndef _MONSTER_AVENGERS_SEARCH_UTIL_
#define _MONSTER_AVENGERS_SEARCH_UTIL_

#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
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
      wprintf(L"] %s %ls (%d)\n", (MELEE == armor.type) ? "--H" : ")->", 
	      armor.name.c_str(), armor_set[i]);
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
  
  struct TreeRoot {
    int id; // OR node id
    std::vector<Signature> jewel_keys;
    
    TreeRoot(int id_) : id(id_), jewel_keys() {}
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
                  NodePool *pool,
                  int effect_id,
                  int skill_id) 
      : pool_(pool), effect_id_(effect_id) {
      armor_points_.resize(data.armors().size());
      int i = 0;
      for (const Armor &armor : data.armors()) {
        armor_points_[i] = 0;
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
      return MaxOr(root.id);
    }

    inline std::vector<int> Split(const TreeRoot &root, int sub_min) {
      std::vector<TempOr> temp_ors;
      std::vector<int> result;
      SplitOr(root.id, sub_min, &temp_ors);
      for (TempOr &item : temp_ors) {
        result.push_back(item.id);
      }
      return result;
    }
    
  private:
    typedef std::unordered_map<int, std::vector<int> > PointsIdListMap;
    
    int MaxArmorOr(int or_id) const {
      int result = -1000;
      for (int armor_id : pool_->Or(or_id).daughters) {
        if (armor_points_[armor_id] > result) {
          result = armor_points_[armor_id];
        }
      }
      return result;
    }

    int MaxAnd(int and_id) const {
      const AND &node = pool_->And(and_id);
      const OR &right_node = pool_->Or(node.right);
      return MaxArmorOr(node.left) + 
        (ANDS == right_node.tag ? MaxOr(node.right) : 
         MaxArmorOr(node.right));
    }
    
    int MaxOr(int or_id) const {
      int result = -1000;
      for (int and_id : pool_->Or(or_id).daughters) {
        int tmp = MaxAnd(and_id);
        if (tmp > result) {
          result = tmp;
        }
      }
      return result;
    }

    int SplitArmorOr(int or_id, PointsIdListMap *new_armors) {
      int result_max = -1000;
      for (int armor_id : pool_->Or(or_id).daughters) {
        if (armor_points_[armor_id] > result_max) {
          result_max = armor_points_[armor_id];
        }
        auto it = new_armors->find(armor_points_[armor_id]);
        if (new_armors->end() != it) {
          it->second.push_back(armor_id);
        } else {
          (*new_armors)[armor_points_[armor_id]] = {armor_id};
        }
      }
      return result_max;
    }

    int SplitArmorOr(int or_id, std::vector<TempOr> *new_armors, int sub_min) {
      PointsIdListMap temp_map;
      int result_max = -1000;
      const OR &node = pool_->Or(or_id);
      for (int armor_id : node.daughters) {
        if (armor_points_[armor_id] > result_max) {
          result_max = armor_points_[armor_id];
        }
        auto it = temp_map.find(armor_points_[armor_id]);
        if (temp_map.end() != it) {
          it->second.push_back(armor_id);
        } else {
          temp_map[armor_points_[armor_id]] = {armor_id};
        }
      }
      
      for (auto &item : temp_map) {
        int new_or_id = pool_->MakeOR<ARMORS>(sig::AddPoints(node.key,
                                                             effect_id_,
                                                             item.first),
                                              &item.second);
        new_armors->emplace_back(new_or_id, item.first);
      }
      
      return result_max;
    }


    void SplitAnd(int and_id, int sub_min, 
                  PointsIdListMap *new_ands) {
      PointsIdListMap split_armors;
      const AND &node = pool_->And(and_id);
      int left_max = SplitArmorOr(node.left, &split_armors);
      std::vector<TempOr> split_right;
      const OR &right_node = pool_->Or(node.right);
      int right_max = (ANDS == right_node.tag) ?
        SplitOr(node.right, sub_min - left_max,
                &split_right) :
        SplitArmorOr(node.right, &split_right, 
                     sub_min - left_max);
      
      const OR &left_node = pool_->Or(node.left);
      for (auto &left_item : split_armors) {
        if (right_max + left_item.first >= sub_min) {
          int left_or_id = 
            pool_->MakeOR<ARMORS>(sig::AddPoints(left_node.key,
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

    int SplitOr(int or_id, int sub_min, std::vector<TempOr> *result) {
      PointsIdListMap new_ands;
      for (int and_id : pool_->Or(or_id).daughters) {
        SplitAnd(and_id, sub_min, &new_ands);
      }
      int result_max = -1000;
      if (!new_ands.empty()) {
        const OR &node = pool_->Or(or_id);
        for (auto &item : new_ands) {
          result->emplace_back(pool_->MakeOR<ANDS>(sig::AddPoints(node.key, 
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
    int effect_id_;
  };
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_SEARCH_UTIL_
