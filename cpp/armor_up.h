#ifndef _MONSTER_AVENGERS_ARMOR_UP_
#define _MONSTER_AVENGERS_ARMOR_UP_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "query.h"
#include "signature.h"
#include "search_util.h"
#include "iterator.h"

namespace monster_avengers {

  class DirectIterator : public IndexIterator {
  public:
    explicit DirectIterator(std::vector<int> &&or_forest)
      : or_forest_() {
      or_forest_.swap(or_forest);
      current_ = or_forest_.begin();
    }

    inline void operator++() override {
      if (or_forest_.end() != current_) current_++;
    }

    inline int operator*() const override {
      return *current_;
    }

    inline bool empty() const override {
      return or_forest_.end() == current_;
    }
    
  private:
    std::vector<int> or_forest_;
    std::vector<int>::const_iterator current_;
  };

  class EffectsIterator : public IndexIterator {
  public:
    explicit EffectsIterator(std::shared_ptr<IndexIterator> base_iter,
                             const NodePool *pool,
                             const std::vector<Effect> &effects)
      : base_iter_(base_iter), pool_(pool), thresholds_() {
      for (const Effect &effect : effects) {
        thresholds_.push_back(effect.points);
      }
      ++(*this);
    }

    inline void operator++() override {
      while (!base_iter_->empty()) {
        int i = **base_iter_;
        std::vector<int> points = 
          std::move(sig::KeyPointsVec(pool_->Or(i).key, 
                                      thresholds_.size()));
        bool satisfied = true;
        int j = 0;
        for (int threshold : thresholds_) {
          wprintf(L"%d\n", points[j]);
          if (points[j++] < threshold) {
            satisfied = false;
            break;
          }            
        }

        if (satisfied) return;
        ++(*base_iter_);
      }
    }

    inline int operator*() const override {
      return **base_iter_;
    }

    inline bool empty() const override {
      return base_iter_->empty();
    }

  private:
    std::shared_ptr<IndexIterator> base_iter_;
    const NodePool *pool_;
    std::vector<int> thresholds_;
  };

  
  class ArmorUp {
  public:
    ArmorUp(const std::string &data_folder) 
      : data_(data_folder), pool_() {
    }

    std::vector<int> SearchFoundation(const Query &query) {
      std::vector<int> previous;
      std::vector<int> current;
      for (ArmorPart part = HEAD; part < PART_NUM; ++part) {
        previous.swap(current);
        current = std::move(ClassifyArmors(part, query));
        if (HEAD != part) {
          current = std::move(MergeForests(current, previous));
        }
      }
      return current;
    }

    void Search(const Query &query, int required_num) {
      std::shared_ptr<DirectIterator> direct;
      direct.reset(new DirectIterator(SearchFoundation(query)));
      std::shared_ptr<EffectsIterator> effects_iter;
      effects_iter.reset(new EffectsIterator(direct.get(),
                                             &pool_,
                                             query.effects));
      ArmorSetIterator output(effects_iter.get(), &pool_); 
      int i = 0;
      while (i < required_num && !output.empty()) {
        const OR &or_node = pool_.Or(output.BaseIndex());
        for (const Effect &effect : sig::KeyEffects(or_node.key, 
                                                    query)) {
          wprintf(L"%ls(%d)  ", 
                  data_.skill_system(effect.skill_id).name.c_str(),
                  effect.points);
        }
        wprintf(L"\n");
        OutputArmorSet(data_, *output);
        ++i;
        ++output;
      }
    }

    // ----- Debug -----
    void Summarize() {
      data_.Summarize();
      Log(INFO, L"OR Nodes: %lld\n", pool_.OrSize());
      Log(INFO, L"AND Nodes: %lld\n", pool_.AndSize());
    }

  private:
    // Returns a vector of newly created or nodes' indices.
    std::vector<int> ClassifyArmors(ArmorPart part, 
                                    const Query &query) {
      std::unordered_map<Signature, std::vector<int> > armor_map;
      bool valid = false;
      for (int id : data_.ArmorIds(part)) {
        const Armor &armor = data_.armor(id);
        if (armor.type == query.weapon_type || BOTH == armor.type) {
          Signature key = sig::ArmorKey(armor, query, &valid);
          if (valid) {
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
          Signature key = sig::CombineKey(left.key, right.key);
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

