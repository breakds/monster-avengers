#ifndef _MONSTER_AVENGERS_CORE_PRUNERS_
#define _MONSTER_AVENGERS_CORE_PRUNERS_

#include <vector>

#include "core/or_and_tree.h"
#include "dataset/core/armor_set.h"
#include "iterators/base.h"
#include "utils/jewels_query.h"
#include "utils/signature.h"

namespace monster_avengers {

struct RawArmorSet {
  std::array<int, PART_NUM> ids;
  std::vector<Signature> jewel_keys;
};

class JewelPruner : public Iterator<TreeRoot> {
public:
  explicit JewelPruner(Iterator<TreeRoot> *source,
                       const NodePool *pool,
                       int effect_id,
                       const std::vector<Effect> &effects, 
                       const JewelFilter &jewel_filter) :
      source_(source), 
      pool_(pool),
      slot_client_({effects[effect_id].id},
                   effects, jewel_filter),
      current_(0),
      inverse_points_(sig::InverseKey(
          effects.begin(),
          effects.begin() + effect_id + 1)) {}

  inline bool Empty() const override {
    return source_->Empty();
  }

  inline bool Next() override {
    current_.jewel_keys.clear();
    while (source_->Next()) {
      const TreeRoot &root = source_->Get();
      current_.id = root.id;
      current_.torso_multiplier = root.torso_multiplier;
      const Signature &key = pool_->Or(current_.id).key;
      if (root.jewel_keys.empty()) {
        const std::unordered_set<Signature> &jewel_keys = 
            slot_client_.Query(key);
        for (const Signature &jewel_key : jewel_keys) {
          if (sig::Satisfy(key | jewel_key, inverse_points_)) {
            current_.jewel_keys.push_back(jewel_key);
          }
        }
      } else {
        int one(0), two(0), three(0), extra(0);
        for (const Signature &existing_key : root.jewel_keys) {
          slot_client_.GetResidual(key, existing_key,
                                   &one, &two, &three, &extra);
          Signature key0 = key | existing_key;
          for (const Signature &jewel_key : 
                   slot_client_.Query(one, two, 
                                      three, extra,
                                      root.torso_multiplier)) {
            if (sig::Satisfy(key0 | jewel_key, inverse_points_)) {
              current_.jewel_keys.push_back(existing_key + jewel_key);
            }
          }
        }
      }
      if (!current_.jewel_keys.empty()) {
        return true;
      }
    }
    return false;
  }

  inline const TreeRoot &Get() const override {
    return current_;
  }

  inline void Reset() override {
    source_->Reset();
  }

 private:
  Iterator<TreeRoot> *source_;
  const NodePool *pool_;
  SlotClient slot_client_;
  TreeRoot current_;
  Signature inverse_points_;
};


class SkillSplitPruner : public Iterator<TreeRoot> {
 public:
  SkillSplitPruner(Iterator<TreeRoot> *source, 
                   const Arsenal &arsenal,
                   NodePool *pool,
                   int effect_id,
                   std::vector<SlotClient> *slot_clients,
                   std::vector<SkillSplitter> *splitters,
                   const Query &query) :
      source_(source), pool_(pool), 
      splitters_(splitters),
      slot_clients_(slot_clients),
      effect_id_(effect_id),
      required_points_(query.effects[effect_id].points),
      inverse_points_(sig::InverseKey(query.effects.begin(), 
                                      query.effects.begin() + effect_id + 1)) {
    std::vector<bool> mask(splitters->size(), false);
    for (int i = 0; i < effect_id; ++i) {
      mask[i] = true;
    }
    per_effect_inverses_.resize(mask.size());
    for (int i = effect_id; i < splitters->size(); ++i) {
      mask[i] = true;
      per_effect_inverses_[i] = sig::InverseKey(query.effects, mask);
      mask[i] = false;
    }
  }

  inline bool Empty() const override {
    return buffer_.empty() && source_->Empty();
  }

  inline bool PreFilter(const TreeRoot &root) {
    int one(0), two(0), three(0), body_holes(0);
    const OR &node = pool_->Or(root.id);
    for (int i = effect_id_ + 1; i < splitters_->size(); ++i) {
      int sub_max = (*splitters_)[i].Max(root);
      Signature key0 = sig::AddPoints(node.key, effect_id_, sub_max);

      bool pass = false;

      for (const Signature &jewel_key : root.jewel_keys) {
        SlotClient::GetResidual(node.key, jewel_key,
                                &one, &two, &three, &body_holes);
        for (const Signature &new_key : (*slot_clients_)[effect_id_].Query(
                 one, two, three, body_holes, root.torso_multiplier)) {
          Signature key1 = jewel_key + new_key;
          if (sig::Satisfy(key0 | key1, per_effect_inverses_[i])) {
            pass = true;
            break;
          }
        }
        if (pass) break;
      }

      if (!pass) return false;
    }

    return true;
  }

  inline bool Next() override {
    if (!buffer_.empty()) buffer_.pop_back();
    if (!buffer_.empty()) return true;
    
    while (source_->Next()) {
      const TreeRoot root = source_->Get();
      std::vector<Signature> jewel_candidates;
      int sub_min = 1000;
        
      if (PreFilter(root)) {
        const OR &node = pool_->Or(root.id);
        int one(0), two(0), three(0), body_holes(0);

        int sub_max = (*splitters_)[effect_id_].Max(root);
        Signature key0 = sig::AddPoints(node.key, effect_id_, sub_max);

        for (const Signature &jewel_key : root.jewel_keys) {
          SlotClient::GetResidual(node.key, jewel_key,
                                  &one, &two, &three, &body_holes);
          for (const Signature &new_key : (*slot_clients_)[effect_id_].Query(
                   one, two, three, body_holes, root.torso_multiplier)) {
            Signature key1 = jewel_key + new_key;
            if (sig::Satisfy(key0 | key1, per_effect_inverses_[effect_id_])) {
              jewel_candidates.push_back(key1);
              int diff = required_points_ - sig::GetPoints(key1, effect_id_);
              if (diff < sub_min) {
                sub_min = diff;
              }
            }
          }
        }
      }
      
      if (!jewel_candidates.empty()) {
        std::vector<int> new_ors =
            (*splitters_)[effect_id_].Split(root, sub_min);
        for (int or_id : new_ors) {
          buffer_.emplace_back(or_id, pool_->Or(or_id));
          const OR &or_node = pool_->Or(or_id);
          for (const Signature &jewel_key : jewel_candidates) {
            if (sig::Satisfy(jewel_key | or_node.key, inverse_points_)) {
              buffer_.back().jewel_keys.push_back(jewel_key);
            }
          }
        }
        return true;
      }
      // If there is no valid jewel signatures, we can proceed to the
      // next tree in the forest.
    }
    return false;
  }

  inline const TreeRoot &Get() const override {
    return buffer_.back();
  }

  inline void Reset() override {
    source_->Reset();
    buffer_.clear();
  }

 private:
  Iterator<TreeRoot> *source_;
  NodePool *pool_;
  std::vector<SlotClient> *slot_clients_;
  std::vector<SkillSplitter> *splitters_;
  int effect_id_;
  int required_points_;
  Signature inverse_points_;
  std::vector<Signature> per_effect_inverses_;
  std::vector<TreeRoot> buffer_;
};

class Unpacker : public Iterator<RawArmorSet> {
 public:
  Unpacker(Iterator<TreeRoot> *source, 
           const NodePool *pool)
      : source_(source), pool_(pool), top_(-1) {}

  inline bool Empty() const override {
    return -1 >= top_ && source_->Empty();
  }
  
  inline bool Next() override {
    int or_id = stack_[top_].or_id;
    while (-1 < top_) {
      const OR &armor_or = pool_->Or(stack_[top_].armor_or_id);
      if ((++stack_[top_].armor_seq) < armor_or.daughters.size()) {
        armor_set_.ids[top_] = armor_or.daughters[stack_[top_].armor_seq];
        if (top_ == PART_NUM - 1) {
          // Simple case: Last level can proceed.
          return true;
        } else {
          or_id = pool_->OrAnd(stack_[top_].or_id, stack_[top_].and_id).right;
          break;
        }
      }
      if (top_ != PART_NUM - 1) {
        const OR &or_node = pool_->Or(stack_[top_].or_id);
        if ((++stack_[top_].and_id) < or_node.daughters.size()) {
          const AND &and_node = 
              pool_->OrAnd(stack_[top_].or_id, stack_[top_].and_id);
          stack_[top_].armor_or_id = and_node.left;
          stack_[top_].armor_seq = 0;
          armor_set_.ids[top_] = pool_->Or(and_node.left).daughters[0];
          or_id = and_node.right;
          break;
        }
      }
      top_--;
    }

    // Most complicated: Source Proceeding.
    if (-1 >= top_) {
      if (source_->Next()) {
        or_id = source_->Get().id;
        armor_set_.jewel_keys = source_->Get().jewel_keys;
      } else {
        return false;
      }
    }

    // Restore stack.
    while (-1 != or_id) {
      const OR &or_node = pool_->Or(or_id);
      if (ANDS == or_node.tag) {
        const AND &and_node = pool_->And(or_node.daughters[0]);
        top_++;
        stack_[top_].or_id = or_id;
        stack_[top_].and_id = 0;
        stack_[top_].armor_or_id = and_node.left;
        stack_[top_].armor_seq = 0;
        armor_set_.ids[top_] = pool_->Or(and_node.left).daughters[0];
        or_id = and_node.right;
      } else {
        top_++;
        stack_[top_].or_id = -1;
        stack_[top_].and_id = -1;
        stack_[top_].armor_or_id = or_id;
        stack_[top_].armor_seq = 0;
        armor_set_.ids[top_] = pool_->Or(or_id).daughters[0];
        or_id = -1;
      }
    }
    return true;
  }

  inline const RawArmorSet& Get() const override {
    return armor_set_;
  }

  inline void Reset() override {
    source_->Reset();
    top_ = -1;
  }

 private:
  struct StackElement {
    int or_id;
    int and_id;
    int armor_or_id;
    int armor_seq;
  };
    
  Iterator<TreeRoot> *source_;
  const NodePool *pool_;
  std::array<StackElement, PART_NUM> stack_;
  int top_;
  RawArmorSet armor_set_;
};

class DefensePruner : public Iterator<RawArmorSet> {
 public:
  DefensePruner(Iterator<RawArmorSet> *source,
                const Arsenal *arsenal,
                int min_defense)
      : source_(source), arsenal_(arsenal),
        min_defense_(min_defense) {}

  inline bool Empty() const override {
    return source_->Empty();
  }

  inline bool Next() override {
    while (source_->Next()) {
      const RawArmorSet &armor_set = source_->Get();
      int defense = 0;
      for (int id : armor_set.ids) defense += (*arsenal_)[id].max_defense;
      if (defense >= min_defense_) {
        return true;
      }
    }
    return false;
  }
  
  inline const RawArmorSet &Get() const override {
    return source_->Get();
  }

  inline void Reset() override {
    source_->Reset();
  }

 private:
  Iterator<RawArmorSet> *source_;
  const Arsenal *arsenal_;
  int min_defense_;
};

// class NegativeSkillPruner : public Iterator<RawArmorSet> {
//  public:
//   NegativeSkillPruner(Iterator<RawArmorSet> *source,
//                       const Arsenal *arsenal)
//       : source_(source), arsenal_(arsenal) {}

//   inline bool Empty() const override {
//     return source_->Empty();
//   }

//   inline bool Next() override {
//     while (source_->Next()) {
//       const RawArmorSet &armor_set = source_->Get();
//       std::vector<Effect> effects = std::move(
//           Data::GetSkillStats(armor_set, *arsenal_));

//       // Checking for negative skills
//       bool pass = true;
//       for (const Effect &effect : effects) {
//         // TODO(breakds): -10 is the simplified threshold. Should be
//         // based on the actual skill data.
//         int threshold = -10;
//         if (effect.points <= threshold) {
//           pass = false;
//           break;
//         }
//       }
//       if (pass) return true;
//     }
//     return false;
//   }
  
//   inline const RawArmorSet &Get() const override {
//     return source_->Get();
//   }

//   inline void Reset() override {
//     source_->Reset();
//   }

//  private:
//   Iterator<RawArmorSet> *source_;
//   const Arsenal *arsenal_;
// };

class Finalizer : public Iterator<ArmorSet> {
 public:
  Finalizer(Iterator<RawArmorSet> *source,
            const Query &query,
            const Arsenal *arsenal,
            int max_per_set)
      : source_(source), arsenal_(arsenal),
        solver_(query.effects, query.jewel_filter),
        max_per_set_(max_per_set), jewel_keys_() {}

  inline bool Empty() const override {
    return jewel_keys_.empty() && source_->Empty();
  }

  inline bool Next() override {
    if (!jewel_keys_.empty()) jewel_keys_.pop_back();
    if (!jewel_keys_.empty()) return true;

    if (source_->Next()) {
      const RawArmorSet &raw = source_->Get();
      for (int i = 0; i < PART_NUM; ++i) {
	current_.ids[i] = raw.ids[PART_NUM - i - 1];
      }
      
      int num_fetched_jewel_plans =
          (std::min)(max_per_set_, static_cast<int>(raw.jewel_keys.size()));

      // TODO(breakds): should rank based on occupied slots.
      for (int i = 0; i < num_fetched_jewel_plans; ++i) {
        jewel_keys_.push_back(raw.jewel_keys[i]);
      }
      multiplier_ = Data::GetMultiplier(current_, *arsenal_);
    }

    if (jewel_keys_.empty()) return false;

    for (int i = 0; i < PART_NUM; ++i) {
      current_.jewels[i].clear();
    }

    // Assign Jewels
    const JewelSolver::JewelPlan jewel_plan = 
	std::move(solver_.Solve(jewel_keys_.back(), multiplier_));

    jewel_keys_.pop_back();

    // Assign body-only jewels.
    if (multiplier_ > 1) {
      for (const auto &item : jewel_plan.second) {
        for (int j = 0; j < item.second; ++j) {
          current_.jewels[BODY].push_back(item.first);
        }
      }
    }

    JewelAssigner assigner(arsenal_);

    for (int i = 0; i < PART_NUM; ++i) {
      if (!(BODY == i && multiplier_ > 1)) {
        assigner.AddPart(i, current_.ids[i]);
      }
    }

    for (const auto &item : jewel_plan.first) {
      assigner.AddJewel(item.first, item.second);
    }

    int part = 0;
    int jewel_id = 0;
    while (assigner.Pop(&part, &jewel_id)) {
      current_.jewels[part].push_back(jewel_id);
    }

    return true;
  }

  inline const ArmorSet &Get() const override {
    return current_;
  }

  inline void Reset() override {
    source_->Reset();
    jewel_keys_.clear();
  }

 private:
  Iterator<RawArmorSet> *source_;
  const Arsenal *arsenal_;
  JewelSolver solver_;
  int max_per_set_;
  ArmorSet current_;
  int multiplier_;
  std::vector<Signature> jewel_keys_;
};

}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_CORE_PRUNERS_
