#ifndef _MONSTER_AVENGERS_ARMOR_UP_
#define _MONSTER_AVENGERS_ARMOR_UP_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "utils/query.h"
#include "utils/signature.h"
#include "or_and_tree.h"
#include "iterator.h"
#include "jewels_query.h"
#include "utils/armor_set.h"

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
    
  private:
    std::vector<TreeRoot> forest_;
    size_t current_;
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
        current_.torso_multiplier = (**base_iter_).torso_multiplier;
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
      return buffer_.empty();
    }
    
  private:
    inline void Proceed() {
      while (!base_iter_->empty()) {
        const TreeRoot root = **base_iter_;
        const OR &node = pool_->Or(root.id);
        int one(0), two(0), three(0);

        int sub_max = splitter_.Max(root);
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
            buffer_.emplace_back(or_id, root.torso_multiplier);
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
      std::array<std::vector<int>, PART_NUM> ordinary_forests;
      for (int part = HEAD; part < PART_NUM; ++part) {
        ordinary_forests[part] = 
          std::move(ClassifyArmors(static_cast<ArmorPart>(part),
                                   query));
      }

      // Forest with armors that provides torso up.
      std::array<std::vector<int>, PART_NUM> torso_up_forests;
      for (int part = HEAD; part <= FEET; ++part) {
        if (BODY != part) {
          torso_up_forests[part] = 
            std::move(ClassifyTorsoUpArmors(static_cast<ArmorPart>(part),
                                            query));          
        }
      }

      // Forest with multiplied armors
      std::array<std::vector<int>, 5> multiplied_forests;
      for (int multiplier = 2; multiplier <= 5; ++multiplier) {
        multiplied_forests[multiplier] =
          std::move(ClassifyArmors(static_cast<ArmorPart>(BODY),
                                   query,
                                   multiplier));
      }

      
      std::vector<int> current;
      std::vector<bool> torso_up(PART_NUM, false);
      std::vector<TreeRoot> result;
      for (unsigned char torso_up_code = 0; 
           torso_up_code < 16; 
           ++torso_up_code) {
        int multiplier = 1;
        multiplier += (torso_up[HEAD] = torso_up_code & 1) ? 1 : 0;
        multiplier += (torso_up[HANDS] = torso_up_code & 2) ? 1 : 0;
        multiplier += (torso_up[WAIST] = torso_up_code & 4) ? 1 : 0;
        multiplier += (torso_up[FEET] = torso_up_code & 8) ? 1 : 0;
        
        current.clear();
        
        for (int part = HEAD; part < PART_NUM; ++part) {
          std::vector<int> &part_forest = 
            (BODY == part && multiplier > 1) ?
            multiplied_forests[multiplier] :
            (torso_up[part] ? torso_up_forests[part] : 
             ordinary_forests[part]);

          if (HEAD == part) {
            current = part_forest;
          } else {
            current = std::move(MergeForests(part_forest, current));
          }
        }

        for (int id : current) {
          result.emplace_back(id, multiplier);
        }
      }
      
      return result;
    }

    void Search(const Query &input_query, int required_num) {
      Query query = OptimizeQuery(input_query);
      query.DebugPrint();
      InitializeExtraArmors(query);
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
        for (const Effect &effect : sig::KeyEffects(or_node.key, 
                                                    query)) {
          wprintf(L"%ls(%d)  ", 
                  data_.skill_system(effect.skill_id).name.c_str(),
                  effect.points);
        }
        int one(0), two(0), three(0);
        sig::KeyHoles(or_node.key, &one, &two, &three);
        wprintf(L"O:%d   OO:%d   OOO:%d\n", one, two, three);

        OutputArmorSet(data_, **output_iterators_.back(), solver);
        ++count;
        ++(*output_iterators_.back());
      }
    }

    void SearchAndOutput(const Query &input_query, int required_num = 10) {
      Query query = OptimizeQuery(input_query);
      InitializeExtraArmors(query);
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
        PrettyPrintArmorSet(data_, **output_iterators_.back(), solver);
	++count;
        ++(*output_iterators_.back());
      }
    }

    void SearchAndLispOut(const Query &input_query, const std::string output_path) {
      Query query = OptimizeQuery(input_query);
      InitializeExtraArmors(query);
      CHECK_SUCCESS(ApplyFoundation(query));
      CHECK_SUCCESS(ApplyJewelFilter(query.effects));
      for (int i = FOUNDATION_NUM; i < query.effects.size(); ++i) {
        CHECK_SUCCESS(ApplySkillSplitter(query, i));	
      }
      CHECK_SUCCESS(PrepareOutput());
      CHECK_SUCCESS(ApplyDefenseFilter(query));
      ArmorSetFormatter formatter(output_path,
                                  &data_, query);
      int count = 0;
      while (count < 10 && !output_iterators_.back()->empty()) {
        const OR &or_node = pool_.Or(output_iterators_.back()->BaseIndex());
        formatter.Format(**output_iterators_.back());
	++count;
        ++(*output_iterators_.back());
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

      // Torso Up Armors
      HoleClient hole_client(data_, query.effects);
      JewelSolver solver(data_, query.effects);
      int body_size = data_.ArmorIds(BODY).size();
      for (int i = 0; i < body_size; ++i) {
        int base_id = data_.ArmorIds(BODY)[i];
        const Armor &armor = data_.armor(BODY, i);
        // Do not include torso_up armors. It typically doesn't exist
        // anyway.
        if (data_.ProvidesTorsoUp(BODY, i)) continue;
        
        int one(0), two(0), three(0);
        switch (armor.holes) {
        case 1: one = 1; break;
        case 2: two = 1; break;
        case 3: three = 1; break;
        }
        const std::unordered_set<Signature> &jewel_keys = 
          hole_client.Query(one, two, three);

        for (const Signature &key : jewel_keys) {
          std::vector<Effect> effects = 
            std::move(sig::KeyEffects(key, query));
          Armor new_armor = data_.armor(BODY, i);
          for (Effect &effect : effects) {
            if (0 == effect.points) continue;
            auto it = std::find_if(new_armor.effects.begin(),
                                   new_armor.effects.end(),
                                   [&effect](const Effect &x) {
                                     return effect.skill_id == x.skill_id;
                                   });
            if (new_armor.effects.end() != it) {
              it->points += effect.points;
            } else {
              new_armor.effects.push_back(effect);
            }
          }
          new_armor.multiplied = true;
          new_armor.holes = 0;
          new_armor.base = base_id;
          new_armor.jewels = solver.Solve(key);
          data_.AddExtraArmor(BODY, new_armor);
        }
      }
    }
    
    // Returns a vector of newly created or nodes' indices.
    std::vector<int> ClassifyArmors(ArmorPart part,
                                    const Query &query,
                                    int multiplier = 1) {
      std::unordered_map<Signature, std::vector<int> > armor_map;

      std::vector<Effect> effects;
      int query_size = query.effects.size();
      for (int i = 0; i < std::min(query_size, FOUNDATION_NUM); ++i) {
        effects.push_back(query.effects[i]);
      }
      
      bool valid = false;
      for (int id : data_.ArmorIds(part)) {
        const Armor &armor = data_.armor(id);
        if (armor.type == query.weapon_type || BOTH == armor.type) {
          Signature key = sig::ArmorKey(armor, effects);
          valid = true;

          if (1 < multiplier) {
            // Torso Up armors, with modified key.
            if (armor.multiplied) {
              sig::KeyMultiplication(&key, multiplier);
            } else {
              valid = false;
            }
          } else {
            // if we are not expecting multiplied armors, filter them.
            valid &= (!armor.multiplied);
          }

          // Rare blacklist
          if (GEAR != part && AMULET != part) {
            valid &= armor.rare >= query.min_rare;
            valid &= armor.rare <= query.max_rare;
          }
          
          // Blacklist filter
          valid &= 0 == query.blacklist.count(id);

          // Weapon holes match
          if (GEAR == part) {
            valid &= armor.holes == query.weapon_holes;
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
      
      std::vector<int> forest;
      forest.reserve(armor_map.size());
      for (auto &item : armor_map) {
        forest.push_back(pool_.MakeOR<ARMORS>(item.first, 
                                              &item.second));
      }
      return forest;
    }

    std::vector<int> ClassifyTorsoUpArmors(ArmorPart part,
                                           const Query &query) {
      std::vector<int> armor_ids;
      for (int i = 0; i < data_.ArmorIds(part).size(); ++i) {
        int id = data_.ArmorIds(part)[i];
        const Armor &armor = data_.armor(id);
        if (data_.ProvidesTorsoUp(part, i) && 
            // Rare blacklist
            armor.rare >= query.min_rare &&
            armor.rare <= query.max_rare &&
            // Blacklist filter
            0 == query.blacklist.count(data_.ArmorIds(part)[i])) {
          armor_ids.push_back(data_.ArmorIds(part)[i]);
        }
      }
      
      std::vector<int> forest;
      if (!armor_ids.empty()) {
        forest.push_back(pool_.MakeOR<ARMORS>(Signature(), &armor_ids));
      }
      return forest;
    }

    std::vector<int> MergeForests(const std::vector<int> &left_ors, 
                                  const std::vector<int> &right_ors) {
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
      iterators_.emplace_back(new ListIterator(Foundation(query)));
      return Status(SUCCESS);
    }

    Status ApplyJewelFilter(const std::vector<Effect> &effects) {
      std::vector<int> skill_ids;
      int actual_size = std::min(static_cast<int>(effects.size()),
                                 FOUNDATION_NUM);
      for (int i = 0; i < actual_size; ++i) {
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

