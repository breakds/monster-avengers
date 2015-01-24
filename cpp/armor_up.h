#ifndef _MONSTER_AVENGERS_ARMOR_UP_
#define _MONSTER_AVENGERS_ARMOR_UP_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "query.h"
#include "signature.h"
#include "or_and_tree.h"
#include "iterator.h"
#include "jewels_query.h"
#include "armor_set.h"

namespace monster_avengers {

  constexpr int FOUNDATION_NUM = 2;

  class DirectIterator : public TreeIterator {
  public:
    explicit DirectIterator(std::vector<int> &&or_forest)
      : forest_() {
      forest_.reserve(or_forest.size());
      for (int or_id : or_forest) {
        forest_.emplace_back(or_id);
      }
      current_ = forest_.begin();
    }

    inline void operator++() override {
      if (forest_.end() != current_) current_++;
    }

    inline const TreeRoot &operator*() const override {
      return *current_;
    }

    inline bool empty() const override {
      return forest_.end() == current_;
    }
    
  private:
    std::vector<TreeRoot> forest_;
    std::vector<TreeRoot>::const_iterator current_;
  };

  class EffectsIterator : public TreeIterator {
  public:
    explicit EffectsIterator(TreeIterator *base_iter,
                             const NodePool *pool,
                             const std::vector<Effect> &effects)
      : base_iter_(base_iter), 
        pool_(pool),
        thresholds_() {
      for (const Effect &effect : effects) {
        thresholds_.push_back(effect.points);
      }
      ++(*this);
    }
    
    inline void operator++() override {
      while (!base_iter_->empty()) {
        const TreeRoot &root = **base_iter_;
        int i = root.id;
        std::vector<int> points = 
          std::move(sig::KeyPointsVec(pool_->Or(i).key, 
                                      thresholds_.size()));
        bool satisfied = true;
        int j = 0;
        for (int threshold : thresholds_) {
          if (points[j++] < threshold) {
            satisfied = false;
            break;
          }            
        }

        if (satisfied) return;
        ++(*base_iter_);
      }
    }

    inline const TreeRoot &operator*() const override {
      return **base_iter_;
    }

    inline bool empty() const override {
      return base_iter_->empty();
    }

  private:
    TreeIterator *base_iter_;
    const NodePool *pool_;
    std::vector<int> thresholds_;
  };

  
  class JewelFilterIterator : public TreeIterator {
  public:
    explicit JewelFilterIterator(TreeIterator *base_iter,
                                 const DataSet &data,
                                 const NodePool *pool,
                                 const std::vector<int> &skill_ids,
                                 const std::vector<Effect> &effects)
      : base_iter_(base_iter), 
        pool_(pool),
        hole_client_(data, skill_ids, effects),
        current_(0),
        inverse_points_(sig::InverseKey(effects.begin(),
                                        effects.begin() + skill_ids.size())) {
      Proceed();
    }

    inline void operator++() override {
      if (!base_iter_->empty()) {
        ++(*base_iter_);
        Proceed();
      }
    }

    inline const TreeRoot &operator*() const override {
      return current_;
    }

    inline bool empty() const override {
      return base_iter_->empty();
    }

  private:
    inline void Proceed() {
      current_.jewel_keys.clear();
      while (!base_iter_->empty()) {
        current_.id = (**base_iter_).id;
        const Signature &key = pool_->Or(current_.id).key;
        const std::unordered_set<Signature> &jewel_keys = 
          hole_client_.Query(key);

        for (const Signature &jewel_key : jewel_keys) {
          if (sig::Satisfy(sig::CombineKeyPoints(key, jewel_key),
                           inverse_points_)) {
            current_.jewel_keys.push_back(jewel_key);
          }
        }
        if (current_.jewel_keys.empty()) {
          ++(*base_iter_);
        } else {
          return;
        }
      }
    }
    
    TreeIterator *base_iter_;
    const NodePool *pool_;
    HoleClient hole_client_;
    TreeRoot current_;
    Signature inverse_points_;
  };

  class SkillSplitIterator : public TreeIterator {
  public:
    SkillSplitIterator(TreeIterator *base_iter, 
                       const DataSet &data,
                       NodePool *pool,
                       int effect_id,
                       const Query &query)
      : base_iter_(base_iter), pool_(pool), 
        splitter_(data, query, pool, effect_id, 
                  query.effects[effect_id].skill_id),
        hole_client_(data, query.effects[effect_id].skill_id, query.effects),
        effect_id_(effect_id),
        required_points_(query.effects[effect_id].points),
        inverse_points_(sig::InverseKey(query.effects.begin(), 
                                        query.effects.begin() + effect_id + 1)) {
      Proceed();
    }

    inline void operator++() override {
      buffer_.pop_back();
      if (buffer_.empty()) {
        ++(*base_iter_);
        Proceed();
      }
    }

    inline const TreeRoot &operator*() const override {
      return buffer_.back();
    }

    inline bool empty() const override {
      return base_iter_->empty();
    }
    
  private:
    inline void Proceed() {
      while (!base_iter_->empty()) {
        const TreeRoot root = **base_iter_;

        const OR &node = pool_->Or(root.id);
        int one(0), two(0), three(0);

        int sub_max = splitter_.Max(root.id);
        int sub_min = 1000;
        Signature key0 = sig::AddPoints(node.key, effect_id_, sub_max);
        
        std::vector<Signature> jewel_candidates;

        for (const Signature &jewel_key : root.jewel_keys) {
          HoleClient::GetResidual(node.key, jewel_key,
                                  &one, &two, &three);
          for (const Signature &new_key : hole_client_.Query(one, 
                                                             two, 
                                                             three)) {
            Signature key1 = sig::CombineKey(jewel_key, new_key);
            if (sig::Satisfy(sig::CombineKeyPoints(key0, key1),
                             inverse_points_)) {
              jewel_candidates.push_back(key1);
              int diff = required_points_ - sig::GetPoints(key1, effect_id_);
              if (diff < sub_min) {
                sub_min = diff;
              }
            }
          }
        }
        
        if (!jewel_candidates.empty()) {
          std::vector<int> new_ors = splitter_.Split(root, sub_min);
          for (int or_id : new_ors) {
            buffer_.emplace_back(or_id);
            const OR &or_node = pool_->Or(or_id);
            for (const Signature &jewel_key : jewel_candidates) {
              if (sig::Satisfy(sig::CombineKeyPoints(jewel_key,
                                                     or_node.key),
                               inverse_points_)) {
                buffer_.back().jewel_keys.push_back(jewel_key);
              }
            }
          }
          break;
        }
        
        // If there is no valid jewel signatures, we can proceed to
        // the next tree in the forest.
        ++(*base_iter_);
      }
    }
      
    TreeIterator *base_iter_;
    NodePool *pool_;
    SkillSplitter splitter_;
    HoleClient hole_client_;
    int effect_id_;
    int required_points_;
    Signature inverse_points_;
    std::vector<TreeRoot> buffer_;
  };

  class DefenseFilterIterator : public ArmorSetIterator {
  public:
    DefenseFilterIterator(ArmorSetIterator *base_iter,
			  const DataSet *data,
			  int min_defense)
      : base_iter_(base_iter), data_(data), min_defense_(min_defense) {
      Proceed();
    }

    void operator++() override {
      ++(*base_iter_);
      Proceed();
    }
    
    const ArmorSet &operator*() const override {
      return **base_iter_;
    }

    bool empty() const override {
      return base_iter_->empty();
    }

    int BaseIndex() const override {
      return base_iter_->BaseIndex();
    }
    
  private:
    void Proceed() {
      while (!base_iter_->empty()) {
	const ArmorSet &armor_set = **base_iter_;
	int defense = 0;
	for (int id : armor_set.ids) defense += data_->armor(id).defense;
	if (defense >= min_defense_) {
	  break;
	} else {
	  ++(*base_iter_);
	}
      }
    }

    ArmorSetIterator *base_iter_;
    const DataSet *data_;
    int min_defense_;
  };




  class ArmorUp {
  public:
    ArmorUp(const std::string &data_folder) 
      : data_(data_folder), pool_(),
        iterators_(), output_iterators_() {}

    std::vector<int> Foundation(const Query &query) {
      std::vector<int> previous;
      std::vector<int> current;
      for (int part = HEAD; part < PART_NUM; ++part) {
        previous.swap(current);
        current = std::move(ClassifyArmors(static_cast<ArmorPart>(part), query));
        if (HEAD != part) {
          current = std::move(MergeForests(current, previous));
        }
      }
      return current;
    }

    void Search(const Query &query, int required_num) {
      CHECK_SUCCESS(ApplyFoundation(query));
      CHECK_SUCCESS(ApplyJewelFilter(query.effects));
      for (int i = FOUNDATION_NUM; i < query.effects.size(); ++i) {
        CHECK_SUCCESS(ApplySkillSplitter(query, i));	
      }
      CHECK_SUCCESS(PrepareOutput());
      CHECK_SUCCESS(ApplyDefenseFilter(query));
      JewelSolver solver(data_, query.effects);
      int count = 0;
      while (count < required_num && !output_iterators_.back()->empty()) {
        wprintf(L"or_node: %d\n", output_iterators_.back()->BaseIndex());
        const OR &or_node = pool_.Or(output_iterators_.back()->BaseIndex());
        for (const Effect &effect : sig::KeyEffects(or_node.key, 
                                                    query)) {
          wprintf(L"%ls(%d)  ", 
                  data_.skill_system(effect.skill_id).name.c_str(),
                  effect.points);
        }
        wprintf(L"\n");
        int one(0), two(0), three(0);
        sig::KeyHoles(or_node.key, &one, &two, &three);
        wprintf(L"O:%d   OO:%d   OOO:%d\n", one, two, three);

        OutputArmorSet(data_, **output_iterators_.back(), query, solver);
	++count;
        ++(*output_iterators_.back());
      }
    }

    void SearchAndOutput(const Query &query, int required_num = 10) {
      CHECK_SUCCESS(ApplyFoundation(query));
      CHECK_SUCCESS(ApplyJewelFilter(query.effects));
      for (int i = FOUNDATION_NUM; i < query.effects.size(); ++i) {
        CHECK_SUCCESS(ApplySkillSplitter(query, i));	
      }
      CHECK_SUCCESS(PrepareOutput());
      CHECK_SUCCESS(ApplyDefenseFilter(query));
      JewelSolver solver(data_, query.effects);
      int count = 0;
      while (count < required_num && !output_iterators_.back()->empty()) {
        const OR &or_node = pool_.Or(output_iterators_.back()->BaseIndex());
        PrettyPrintArmorSet(data_, **output_iterators_.back(), query, solver);
	++count;
        ++(*output_iterators_.back());
      }
    }

    void ListSkills() {
      data_.PrintSkillSystems();
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

      std::vector<Effect> effects;
      for (int i = 0; i < FOUNDATION_NUM; ++i) {
        effects.push_back(query.effects[i]);
      }
      
      bool valid = false;
      for (int id : data_.ArmorIds(part)) {
        const Armor &armor = data_.armor(id);
        if (armor.type == query.weapon_type || BOTH == armor.type) {
          Signature key = sig::ArmorKey(armor, effects, &valid);

          if (GEAR == part && armor.holes == query.weapon_holes) {
	    // Weapon whitelist
            valid = true;
          } else if (AMULET == part) {
            valid = true;
          } else if (armor.rare < query.min_rare) {
	    // Rare blacklist
	    valid = false;
	  }
	  
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
      
      // This is only for amulets now
      if (AMULET == part) {
        for (int i = 0; i < query.amulets.size(); ++i) {
          Signature key = sig::ArmorKey(query.amulets[i], effects, &valid);
          if (valid || query.amulets[i].holes > 0) {
            auto it = armor_map.find(key);
            int id = data_.armors().size() + i;
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

    Status ApplyFoundation(const Query &query) {
      iterators_.clear();
      iterators_.emplace_back(new DirectIterator(Foundation(query)));
      return Status(SUCCESS);
    }

    Status ApplyJewelFilter(const std::vector<Effect> &effects) {
      std::vector<int> skill_ids;
      for (int i = 0; i < FOUNDATION_NUM; ++i) {
        skill_ids.push_back(effects[i].skill_id);
      }
      TreeIterator *new_iter = 
        new JewelFilterIterator(iterators_.back().get(),
                                data_,
                                &pool_,
                                skill_ids,
                                effects);
      iterators_.emplace_back(new_iter);
      return Status(SUCCESS);
    }

    Status ApplySkillSplitter(const Query &query,
                              int effect_id) {
      TreeIterator *new_iter = 
        new SkillSplitIterator(iterators_.back().get(),
                               data_,
                               &pool_,
                               effect_id,
                               query);
      iterators_.emplace_back(new_iter);
      return Status(SUCCESS);
    }

    Status PrepareOutput() {
      output_iterators_.emplace_back(new ExpansionIterator(iterators_.back().get(), 
							   &pool_));
      return Status(SUCCESS);
    }

    Status ApplyDefenseFilter(const Query &query) {
      output_iterators_.emplace_back(new DefenseFilterIterator(output_iterators_.back().get(),
							       &data_,
							       query.defense));
      return Status(SUCCESS);
    }
    
    DataSet data_;
    NodePool pool_;
    std::vector<std::unique_ptr<TreeIterator> > iterators_;
    std::vector<std::unique_ptr<ArmorSetIterator> > output_iterators_;
  };
}

#endif  // _MONSTER_AVENGERS_ARMOR_UP_

