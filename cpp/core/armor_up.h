#ifndef _MONSTER_AVENGERS_ARMOR_UP_
#define _MONSTER_AVENGERS_ARMOR_UP_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "aux/timer.h"
#include "utils/query.h"
#include "utils/signature.h"
#include "utils/jewels_query.h"
#include "utils/formatter.h"
#include "or_and_tree.h"
#include "iterator.h"
#include "explore.h"

namespace monster_avengers {

  constexpr int FOUNDATION_NUM = 2;

  class ListIterator : public TreeIterator {
  public:
    explicit ListIterator(const std::vector<TreeRoot> &&input) 
      : forest_(input), current_(0) {}
    
    inline void operator++() override {
      if (current_ < forest_.size()) current_++;
    }

    inline const TreeRoot &operator*() const override {
      return forest_[current_];
    }
    
    inline bool empty() const override {
      return current_ >= forest_.size();
    }

    inline void Reset() override {}
    
  private:
    std::vector<TreeRoot> forest_;
    size_t current_;
  };

  class JewelFilterIterator : public TreeIterator {
  public:
    explicit JewelFilterIterator(TreeIterator *base_iter,
                                 const DataSet &data,
                                 const NodePool *pool,
                                 int effect_id,
                                 const std::vector<Effect> &effects)
      : base_iter_(base_iter), 
        pool_(pool),
        hole_client_(data, {effects[effect_id].skill_id}, effects),
        current_(0),
        inverse_points_(sig::InverseKey(effects.begin(),
                                        effects.begin() + effect_id + 1)) {
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

    inline void Reset() override {}

  private:
    inline void Proceed() {
      current_.jewel_keys.clear();
      while (!base_iter_->empty()) {
        const TreeRoot &root = **base_iter_;
        current_.id = root.id;
        current_.torso_multiplier = root.torso_multiplier;
        const Signature &key = pool_->Or(current_.id).key;
        if (root.jewel_keys.empty()) {
          const std::unordered_set<Signature> &jewel_keys = 
            hole_client_.Query(key);
          for (const Signature &jewel_key : jewel_keys) {
            if (sig::Satisfy(key | jewel_key, inverse_points_)) {
              current_.jewel_keys.push_back(jewel_key);
            }
          }
        } else {
          int one(0), two(0), three(0), extra(0);
          for (const Signature &existing_key : root.jewel_keys) {
            hole_client_.GetResidual(key, existing_key,
                                     &one, &two, &three, &extra);
            Signature key0 = key | existing_key;
            for (const Signature &jewel_key : 
                   hole_client_.Query(one, two, 
                                      three, extra,
                                      root.torso_multiplier)) {
              if (sig::Satisfy(key0 | jewel_key, inverse_points_)) {
                current_.jewel_keys.push_back(existing_key + jewel_key);
              }
            }
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
        splitter_(data, pool, effect_id, 
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
      return buffer_.empty();
    }

    inline void Reset() override {}
    
  private:
    inline void Proceed() {
      while (!base_iter_->empty()) {
        const TreeRoot root = **base_iter_;
        const OR &node = pool_->Or(root.id);
        int one(0), two(0), three(0), body_holes(0);

        int sub_max = splitter_.Max(root);
        int sub_min = 1000;
        Signature key0 = sig::AddPoints(node.key, effect_id_, sub_max);
        std::vector<Signature> jewel_candidates;

        for (const Signature &jewel_key : root.jewel_keys) {
          HoleClient::GetResidual(node.key, jewel_key,
                                  &one, &two, &three, &body_holes);
          for (const Signature &new_key : 
                 hole_client_.Query(one, two, three, 
                                    body_holes, root.torso_multiplier)) {
            Signature key1 = jewel_key + new_key;
            if (sig::Satisfy(key0 | key1, inverse_points_)) {
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
            buffer_.emplace_back(or_id, pool_->Or(or_id));
            const OR &or_node = pool_->Or(or_id);
            for (const Signature &jewel_key : jewel_candidates) {
              if (sig::Satisfy(jewel_key | or_node.key, inverse_points_)) {
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
	for (int id : armor_set.ids) defense += data_->armor(id).max_defense;
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
    
    std::vector<TreeRoot> Foundation(const Query &query) {

      // Forest with no torso up.
      std::array<std::vector<int>, PART_NUM> part_forests;
      for (int part = HEAD; part < PART_NUM; ++part) {
        part_forests[part] = 
          std::move(ClassifyArmors(static_cast<ArmorPart>(part),
                                   query));
      }
      
      std::vector<int> current;
      for (int part = HEAD; part < PART_NUM; ++part) {
        if (HEAD == part) {
          current = part_forests[part];
        } else {
          current = std::move(MergeForests(part_forests[part], current,
                                           part == BODY));
        }
      }

      std::vector<TreeRoot> result;
      
      for (int id : current) {
        result.emplace_back(id, pool_.Or(id));
      }

      return result;
    }

    void Search(const Query &input_query, 
                const std::string output_path) {
      bool to_screen = "" == output_path;
      
      // Optimize the Query
      Query query = OptimizeQuery(input_query);

      // Add in custom armors
      InitializeExtraArmors(query);

      // Core Search
      CHECK_SUCCESS(ApplyFoundation(query));
      for (int i = 0; i < FOUNDATION_NUM; ++i) {
        CHECK_SUCCESS(ApplySingleJewelFilter(query.effects, i));
      }
      for (int i = FOUNDATION_NUM; i < query.effects.size(); ++i) {
        CHECK_SUCCESS(ApplySkillSplitter(query, i));	
      }
      CHECK_SUCCESS(PrepareOutput());
      CHECK_SUCCESS(ApplyDefenseFilter(query));
      
      // Prepare formatter
      ArmorSetFormatter formatter(output_path, &data_, query);
      
      int count = 0;
      while (count < query.max_results && !output_iterators_.back()->empty()) {
        const OR &or_node = pool_.Or(output_iterators_.back()->BaseIndex());
        formatter(**output_iterators_.back());
	++count;
        ++(*output_iterators_.back());
      }
    }

    // Iterate is for speed test only.
    void Iterate(const Query &input_query) {
      // Optimize the Query
      Query query = OptimizeQuery(input_query);

      // Add in custom armors
      InitializeExtraArmors(query);

      // Core Search
      CHECK_SUCCESS(ApplyFoundation(query));
      CHECK_SUCCESS(ApplySingleJewelFilter(query.effects, 0));
      CHECK_SUCCESS(ApplySingleJewelFilter(query.effects, 1));
      for (int i = FOUNDATION_NUM; i < query.effects.size(); ++i) {
        CHECK_SUCCESS(ApplySkillSplitter(query, i));	
      }
      CHECK_SUCCESS(PrepareOutput());
      CHECK_SUCCESS(ApplyDefenseFilter(query));
      
      // Prepare formatter
      while (!output_iterators_.back()->empty()) {
        ++(*output_iterators_.back());
      }
    }

    void Explore(const Query &input_query,
                 const std::string output_path = "") {
      // Optimize the Query
      Query query = OptimizeQuery(input_query);

      // Add in custom armors
      InitializeExtraArmors(query);

      // Core Search
      CHECK_SUCCESS(ApplyFoundation(query));
      for (int i = 0; i < FOUNDATION_NUM; ++i) {
        CHECK_SUCCESS(ApplySingleJewelFilter(query.effects, i));
      }
      for (int i = FOUNDATION_NUM; i < query.effects.size(); ++i) {
        CHECK_SUCCESS(ApplySkillSplitter(query, i));	
      }
      
      ExploreResult result;

      Timer timer;
      timer.Tic();
      CachedTreeIterator iterator(iterators_.back().get());
      wprintf(L"Initialization: %.4lf\n", timer.Toc());
      
      pool_.PushSnapshot();
      for (int i = 1; i < data_.skill_systems().size(); ++i) {
        pool_.RestoreSnapshot();
        timer.Tic();
        auto it = std::find_if(query.effects.begin(),
                               query.effects.end(),
                               [&i](const Effect &effect) {
                                 return effect.skill_id == i;
                               });
        if (query.effects.end() == it) {
          if (ExploreSkill(&iterator, data_, &pool_, i, query.effects)) {
            result.ids.push_back(i);
            if ("" == output_path) {
              wprintf(L"[%03d] %.4lf %ls: PASS ^_^ \n", 
                      i,
                      timer.Toc(),
                      data_.skill_system(i).name.c_str());
            }
          } else if ("" == output_path) {
            wprintf(L"[%03d] %.4lf %ls: fail\n", 
                    i,
                    timer.Toc(),
                    data_.skill_system(i).name.c_str());
          }
        }
      }
      
      if ("" != output_path) {
        std::wofstream output_stream(output_path);
        if (!output_stream.good()) {
          Log(ERROR, L"error while opening %s.", output_path.c_str());
          exit(-1);
        }
        output_stream.imbue(std::locale("en_US.UTF-8"));
        output_stream << result.Format() << "\n";
      }
    }

    Query OptimizeQuery(const Query &query) {
      std::vector<double> scores;
      std::vector<int> indices;
      for (int i = 0; i < query.effects.size(); ++i) {
        const Effect &effect = query.effects[i];
        indices.push_back(i);
        scores.push_back(data_.EffectScore(effect));
        wprintf(L"%ls: %.5lf\n", 
                data_.skill_system(effect.skill_id).name.c_str(),
                scores.back());
      }

      std::sort(indices.begin(), indices.end(), 
                [&scores](int a, int b) {
                  return scores[a] < scores[b];
                });
      Query optimized = query;
      optimized.effects.clear();
      for (int i = 0; i < query.effects.size(); ++i) {
        optimized.effects.push_back(query.effects[indices[i]]);
      }
      return optimized;
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
    void InitializeExtraArmors(const Query &query) {
      data_.ClearExtraArmor();
      // Amulets
      for (const Armor &amulet : query.amulets) {
        data_.AddExtraArmor(AMULET, amulet);
      }
    }
    
    // Returns a vector of newly created or nodes' indices.
    std::vector<int> ClassifyArmors(ArmorPart part,
                                    const Query &query) {
      std::unordered_map<Signature, std::vector<int> > armor_map;

      std::vector<Effect> effects;
      int query_size = query.effects.size();
      for (int i = 0; i < std::min(query_size, FOUNDATION_NUM); ++i) {
        effects.push_back(query.effects[i]);
      }
      
      for (int id : data_.ArmorIds(part)) {
        const Armor &armor = data_.armor(id);
        if (armor.type == query.weapon_type || BOTH == armor.type) {
          Signature key(armor, effects);
          
          // Rare blacklist
          if (GEAR != part && AMULET != part) {
            if (armor.rare < query.min_rare ||
                armor.rare > query.max_rare) continue;
          }
          
          // Blacklist filter
          if (0 != query.blacklist.count(id)) continue;
          
          // Weapon holes match
          if (GEAR == part && armor.holes != query.weapon_holes) continue;
          
          auto it = armor_map.find(key);
          if (armor_map.end() == it) {
            armor_map[key] = {id};
          } else {
            it->second.push_back(id);
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

    std::vector<int> MergeForests(const std::vector<int> &left_ors, 
                                  const std::vector<int> &right_ors, 
                                  bool is_body = false) {
      std::unordered_map<Signature, std::vector<int> > and_map;
      for (int i : left_ors) {
        const OR &left = pool_.Or(i);
        for (int j : right_ors) {
          const OR &right = pool_.Or(j);
          Signature key = left.key;
          if (is_body) {
            key.BodyRefactor(right.key.multiplier() + 1);
          }
          key += right.key;
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
      iterators_.emplace_back(new ListIterator(Foundation(query)));
      return Status(SUCCESS);
    }

    // Status ApplyJewelFilter(const std::vector<Effect> &effects) {
    //   std::vector<int> skill_ids;
    //   int actual_size = std::min(static_cast<int>(effects.size()),
    //                              FOUNDATION_NUM);
    //   for (int i = 0; i < actual_size; ++i) {
    //     skill_ids.push_back(effects[i].skill_id);
    //   }
    //   TreeIterator *new_iter = 
    //     new JewelFilterIterator(iterators_.back().get(),
    //                             data_,
    //                             &pool_,
    //                             skill_ids,
    //                             effects);
    //   iterators_.emplace_back(new_iter);
    //   return Status(SUCCESS);
    // }

    Status ApplySingleJewelFilter(const std::vector<Effect> &effects, 
                                  int effect_id) {
      TreeIterator *new_iter = 
        new JewelFilterIterator(iterators_.back().get(),
                                data_,
                                &pool_,
                                effect_id,
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

