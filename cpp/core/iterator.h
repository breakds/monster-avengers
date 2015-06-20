#ifndef _MONSTER_AVENGERS_ITERATOR_
#define _MONSTER_AVENGERS_ITERATOR_

#include <memory>
#include <array>

#include "dataset/core/armor_set.h"
#include "or_and_tree.h"
#include "utils/jewels_query.h"

using monster_avengers::dataset::ArmorSet;

namespace monster_avengers {

struct RawArmorSet {
  std::array<int, PART_NUM> ids;
  std::vector<Signature> jewel_keys;
};

class TreeIterator {
 public:
  virtual void operator++() = 0;
  virtual const TreeRoot &operator*() const = 0;
  virtual bool empty() const = 0;
  virtual void Reset() = 0;
};

class ArmorSetIterator {
 public:
  virtual void operator++() = 0;
  virtual const RawArmorSet& operator*() const = 0;
  virtual bool empty() const = 0;
  virtual int BaseIndex() const = 0;
  // virtual void Reset() = 0;
};

class FinalizeIterator {
 public:
  FinalizeIterator(ArmorSetIterator *base_iter,
                   const Query &query,
                   const Arsenal *arsenal,
                   int max_per_set)
      : base_iter_(base_iter), arsenal_(arsenal),
        solver_(query.effects, query.jewel_filter),
        max_per_set_(max_per_set), jewel_keys_(), is_empty_(false) {
    (operator++)();
  }

  inline void operator++() {
    if (jewel_keys_.empty()) {
      Proceed();
      ++(*base_iter_);
    }
    if (!jewel_keys_.empty()) {
      for (int i = 0; i < PART_NUM; ++i) {
        current_.jewels[i].clear();
      }
    } else {
      is_empty_ = true;
      return;
    }

    // Assign Jewels
    const JewelSolver::JewelPlan jewel_plan = 
	std::move(solver_.Solve(jewel_keys_.back(), multiplier_));

    jewel_keys_.pop_back();

    // Assign body-only jewels.
    // TODO(breakds): Should use a Data static function.
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
  }

  inline const ArmorSet &operator*() const {
    return current_;
  }

  inline bool empty() const {
    return is_empty_;
  }

  inline void Reset() {}

 private:
  inline void Proceed() {
    if (!base_iter_->empty()) {
      const RawArmorSet &raw = **base_iter_;
      for (int i = 0; i < PART_NUM; ++i) {
	current_.ids[i] = raw.ids[PART_NUM - i - 1];
      }

      int num_fetched_jewel_plans =
          std::min(max_per_set_, static_cast<int>(raw.jewel_keys.size()));

      // TODO(breakds): should rank based on occupied slots.
      for (int i = 0; i < num_fetched_jewel_plans; ++i) {
        jewel_keys_.push_back(raw.jewel_keys[i]);
      }

      multiplier_ = std::accumulate(
          current_.ids.begin(),
          current_.ids.end(),
          1, // initially we have 1 for body itself
          [&](int accu, int id) {
            return accu + (arsenal_->IsTorsoUp(id) ? 1 : 0);
          });
    }
  }

  
  ArmorSetIterator *base_iter_;
  const Arsenal *arsenal_;
  JewelSolver solver_;
  int max_per_set_;
  ArmorSet current_;
  int multiplier_;
  std::vector<Signature> jewel_keys_;
  bool is_empty_;
};


class ExpansionIterator : public ArmorSetIterator {
 public:
  ExpansionIterator(TreeIterator *base_iter, 
                    const NodePool *pool)
      : base_iter_(base_iter), pool_(pool), top_(-1) {
    if (!base_iter_->empty()) {
      int or_id = (**base_iter).id;
      armor_set_.jewel_keys = (**base_iter).jewel_keys;
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
    }
  }

  void operator++() override {
    if (-1 >= top_) return;
    {
      const OR &armor_or = pool_->Or(stack_[top_].armor_or_id);
      if ((++stack_[top_].armor_seq) < armor_or.daughters.size()) {
        armor_set_.ids[top_] = armor_or.daughters[stack_[top_].armor_seq];
        return;
      } 
    }
      
    top_--;
    int or_id = stack_[top_].or_id;
    while (0 <= top_) {
      const OR &armor_or = pool_->Or(stack_[top_].armor_or_id);
      if ((++stack_[top_].armor_seq) < armor_or.daughters.size()) {
        armor_set_.ids[top_] = armor_or.daughters[stack_[top_].armor_seq];
        or_id = pool_->OrAnd(stack_[top_].or_id, stack_[top_].and_id).right;
        break;
      }
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
      top_--;
    }
      
    if (-1 >= top_) {
      if (!base_iter_->empty()) {
        ++(*base_iter_);
        if (!base_iter_->empty()) {
          or_id = (**base_iter_).id;
          armor_set_.jewel_keys = (**base_iter_).jewel_keys;
        } else {
          or_id = -1;
        }
      } else {
        or_id = -1;
      }
    }
      
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
      
    return;
  }

  inline const RawArmorSet& operator*() const override {
    return armor_set_;
  }

  inline bool empty() const override {
    return -1 >= top_;
  }

  inline int BaseIndex() const override {
    return (**base_iter_).id;
  }
    
 private:
  struct StackElement {
    int or_id;
    int and_id;
    int armor_or_id;
    int armor_seq;
  };
    
  TreeIterator *base_iter_;
  const NodePool *pool_;
  std::array<StackElement, PART_NUM> stack_;
  int top_;
  RawArmorSet armor_set_;
};
}

#endif  // _MONSTER_AVENGERS_ITERATOR_
