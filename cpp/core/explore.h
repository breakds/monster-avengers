#ifndef _MONSTER_AVENGERS_EXPLORE_
#define _MONSTER_AVENGERS_EXPLORE_

#include <vector>

#include "data/data_set.h"
#include "utils/signature.h"
#include "utils/jewels_query.h"
#include "iterator.h"

namespace monster_avengers {
  
  class CachedTreeIterator : public TreeIterator {
  public:
    explicit CachedTreeIterator(TreeIterator *base_iter) 
      : cache_(), current_(0) {
      while (!base_iter->empty()) {
        cache_.push_back(**base_iter);
        ++(*base_iter);
      }
    }

    inline void operator++() override {
      if (current_ < cache_.size()) current_++;
    }

    inline const TreeRoot &operator*() const override {
      return cache_[current_];
    }

    inline bool empty() const override {
      return current_ >= cache_.size();
    }

    inline void Reset() override {
      current_ = 0;
    }

  private:
    std::vector<TreeRoot> cache_;
    size_t current_;
  };

  bool ExploreSkill(CachedTreeIterator *iterator,
                    const DataSet &data, 
                    NodePool *pool, 
                    int skill_id, 
                    const std::vector<Effect> &previous_effects) {
    std::vector<Effect> effects = previous_effects;
    effects.emplace_back(skill_id, 
                         data.skill_system(skill_id).LowestPositivePoints());
    int effect_id = effects.size() - 1;
    
    // Construct the hole client
    HoleClient hole_client(data, skill_id, effects);

    // Construct the splitter.
    SkillSplitter splitter(data, pool, effect_id, skill_id);

    Signature inverse_points(sig::InverseKey(effects.begin(), 
                                             effects.end()));

    int one(0), two(0), three(0), body_holes(0);
    
    iterator->Reset();
    while (!iterator->empty()) {
      const TreeRoot &root = **iterator;
      const OR &node = pool->Or(root.id);
      int sub_max = splitter.Max(root);
      Signature key0 = sig::AddPoints(node.key, effect_id, sub_max);
      
      for (const Signature &jewel_key : root.jewel_keys) {
        HoleClient::GetResidual(node.key, jewel_key,
                                &one, &two, &three, &body_holes);
        for (const Signature &new_key : 
               hole_client.Query(one, two, three, 
                                 body_holes, root.torso_multiplier)) {
          Signature key1 = jewel_key + new_key;
          if (sig::Satisfy(key0 | key1, inverse_points)) {
            return true;
          }
        }
      }
      ++(*iterator);
    }
    return false;
  }
  
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_EXPLORE_
