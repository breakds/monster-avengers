#ifndef _MONSTER_AVENGERS_ITERATOR_
#define _MONSTER_AVENGERS_ITERATOR_

#include <memory>
#include <array>
#include "or_and_tree.h"

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
