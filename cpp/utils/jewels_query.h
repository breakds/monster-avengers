#ifndef _MONSTER_AVENGERS_UTILS_JEWELS_QUERY_
#define _MONSTER_AVENGERS_UTILS_JEWELS_QUERY_

#include <array>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include "dataset/dataset.h"
#include "utils/filter.h"
#include "utils/signature.h"

namespace monster_avengers {

using dataset::Data;

class SlotClient {
 public:
  const static int MAX_ONES = 24;
  const static int MAX_TWOS = 8;
  const static int MAX_THREES = 8;

  SlotClient(const std::vector<int> &skill_ids,
             const std::vector<Effect> &effects, 
             const JewelFilter &filter)
      : jewel_keys_() {
    bool valid = false;

    for (int i = 0; i < Data::jewels().size(); ++i) {
      const Jewel &jewel = Data::jewel(i);
      if (filter.Validate(i)) {
        Signature key = Signature(jewel, skill_ids, 
                                  effects, &valid);
        if (valid) {
          jewel_keys_[jewel.slots].insert(key);
        }
      }
    }

    buffer_[0].insert(Signature());
    fixed_buffer_[1][0].insert(Signature());
    fixed_buffer_[2][0].insert(Signature());
    fixed_buffer_[3][0].insert(Signature());
  }
    
  SlotClient(const std::vector<Effect> &effects, 
             const JewelFilter &filter) 
      : SlotClient(SkillIdsFromEffects(effects), effects, filter) {}
    
  SlotClient(int skill_id, 
             const std::vector<Effect> &effects, 
             const JewelFilter &filter) 
      : SlotClient(std::vector<int>({skill_id}), 
                   effects, filter) {}
    
  inline const std::unordered_set<Signature> &Query(Signature input) {
    int i(0), j(0), k(0);
    sig::KeySlots(input, &i, &j, &k);
    return Calculate(i, j, k, input.BodySlotSum(), input.multiplier());
  }

  inline const std::unordered_set<Signature> &Query(int i, 
                                                    int j, 
                                                    int k,
                                                    int extra,
                                                    int multiplier) {
    return Calculate(i, j, k, extra, multiplier);
  }

  // Use the hole aligment from stuffed to stuff the original hole
  // aligment, and get the residual hole alignment.
  static void GetResidual(const Signature &original, 
                          const Signature &stuffed,
                          int *i, int *j, int *k, int *extra) {
    sig::KeySlots(original, i, j, k);
    int one(0), two(0), three(0);
    sig::KeySlots(stuffed, &one, &two, &three);

    // Handle 3 slots
    *k -= three;

    // Handle 2 slots
    if (two <= *j) {
      *j -= two;
    } else {
      two -= *j;
      *j = 0;
      *k -= two;
      *i += two;
    }

    if (one <= *i) {
      *i -= one;
    } else {
      one -= *i;
      *i = 0;
      if (one <= ((*j) << 1)) {
        *j -= ((one + 1) >> 1);
        if (1 & one) {
          *i = 1;
        }
      } else {
        one -= (*j << 1);
        *j = 0;
        *k -= ((one + 2) / 3);
        int remain = one % 3;
        if (1 == remain) {
          (*j)++;
        } else if (2 == remain) {
          (*i)++;
        }
      }
    }

    // Handle Extra:
    *extra = original.BodySlotSum() - stuffed.BodySlotSum();
  }

  // This is only for unit test purpose.
  std::unordered_set<Signature> DFS(int i, int j, int k) {
    std::array<std::vector<Signature>, 4> jewels;
    for (int slots = 1; slots <= 3; ++slots) {
      for (const Signature &key : jewel_keys_[slots]) {
        jewels[slots].push_back(key);
      }
    }
    std::unordered_set<Signature> result;
    DFS(i, j, k, 3, 0, Signature(), jewels, &result);
    return result;
  }

 private:
  std::vector<int> SkillIdsFromEffects(const std::vector<Effect> &effects) {
    std::vector<int> skill_ids;
    for (const Effect &effect : effects) {
      skill_ids.push_back(effect.id);
    }
    return skill_ids;
  }

  inline void SetProduct(const std::unordered_set<Signature> &a,
                         const std::unordered_set<Signature> &b,
                         std::unordered_set<Signature> *c) {
    for (const Signature &key_a : a) {
      for (const Signature &key_b : b) {
        c->insert(key_a + key_b);
      }
    }
  }

  inline void SetUnion(const std::unordered_set<Signature> &input,
                       std::unordered_set<Signature> *base) {
    for (const Signature &key : input) {
      base->insert(key);
    }
  }

  const std::unordered_set<Signature> &CalculateFixed(int slots, int i) {
    if (!fixed_buffer_[slots][i].empty()) {
      return fixed_buffer_[slots][i];
    }
    SetProduct(CalculateFixed(slots, i - 1),
               jewel_keys_[slots],
               &fixed_buffer_[slots][i]);
    return fixed_buffer_[slots][i];
  }

  const std::unordered_set<Signature> &Calculate(int i) {
    if (!buffer_[i].empty()) {
      return buffer_[i];
    }

    buffer_[i] = CalculateFixed(1, i);
    SetUnion(Calculate(i - 1),
             &buffer_[i]);
    return buffer_[i];
  }

  const std::unordered_set<Signature> &Calculate(int i, int j) {
    int index = j * MAX_ONES + i;
      
    if (!buffer_[index].empty()) {
      return buffer_[index];
    }
      
    if (0 == j) {
      return Calculate(i);
    }

    SetProduct(Calculate(i),
               CalculateFixed(2, j),
               &buffer_[index]);
    SetUnion(Calculate(i + 2, j - 1),
             &buffer_[index]);
      
    return buffer_[index];
  }

  const std::unordered_set<Signature> &Calculate(int i, int j, int k) {
    int index = (k * MAX_TWOS + j) * MAX_ONES + i;
    if (!buffer_[index].empty()) {
      return buffer_[index];
    }
        
    if (0 == j && 0 == k) {
      return Calculate(i);
    }

    if (0 == k) {
      return Calculate(i, j);
    }

    SetProduct(Calculate(i, j), CalculateFixed(3, k),
               &buffer_[index]);
    SetUnion(Calculate(i + 1, j + 1, k - 1),
             &buffer_[index]);
    return buffer_[index];
  }

  const std::unordered_set<Signature> &Calculate(int i, int j, int k, 
                                                 int extra, int multiplier) {
    const std::unordered_set<Signature> &base_answer = Calculate(i, j, k);
    if (2 > multiplier || 0 == extra) {
      return base_answer;
    }

    int index = ((((multiplier -1) * 3 + (extra - 1)) * MAX_THREES + k) * MAX_TWOS 
                 + j) * MAX_ONES + i;

    if (!buffer_[index].empty()) {
      return buffer_[index];
    }
      
    const std::unordered_set<Signature> &extension = 
        1 == extra ? Calculate(1, 0, 0) :
        (2 == extra ? Calculate(0, 1, 0) : Calculate(0, 0, 1));
      
    std::unordered_set<Signature> transformed;
      
    for (Signature key : extension) {
      key.BodyRefactor(multiplier);
      transformed.insert(key);
    }
      
    SetProduct(base_answer, transformed, &buffer_[index]);
    return buffer_[index];
  }

  void DFS(int i, int j, int k, int slots, int id, Signature key,
           const std::array<std::vector<Signature>, 4> &jewels,
           std::unordered_set<Signature> *result) {
    result->insert(key);
    if (0 == i + j + k) {
      return;
    }
    int p = slots;
    int q = id;
    do {
      if (1 == p) {
        if (0 < i) {
          DFS(i - 1, j, k, p, q, 
              key + jewels[p][q],
              jewels, result);
        } else if (0 < j) {
          DFS(i + 1, j - 1, k, p, q,
              key + jewels[p][q],
              jewels, result);
        } else {
          DFS(i, j + 1, k - 1, p, q,
              key + jewels[p][q],
              jewels, result);
        }
      } else if (2 == p) {
        if (0 < j) {
          DFS(i, j - 1, k, p, q,
              key + jewels[p][q],
              jewels, result);
        } else if (0 < k) {
          DFS(i + 1, j, k - 1, p, q,
              key + jewels[p][q],
              jewels, result);
        }
      } else if (3 == p) {
        if (0 < k) {
          DFS(i, j, k - 1, p, q,
              key + jewels[p][q],
              jewels, result);
        }
      }

      q++;
      if (jewels[p].size() <= q) {
        p--;
        q = 0;
      }
    } while (p > 0);
  }

    
  std::array<std::unordered_set<Signature>, 4> jewel_keys_;
  std::array<std::array<std::unordered_set<Signature>, 
                        MAX_ONES>, 4> fixed_buffer_;
  std::array<std::unordered_set<Signature>, 
             MAX_ONES * MAX_TWOS * MAX_THREES * 3 * 5> buffer_;
};


class JewelSolver {
 public:
  typedef std::pair<std::unordered_map<int, int>, 
                    std::unordered_map<int, int> > JewelPlan;

    
  JewelSolver(const std::vector<Effect> &effects, 
              const JewelFilter &filter)
      : jewel_keys_() {
    bool valid = false;
    std::vector<int> skill_ids;
    for (const Effect &effect : effects) {
      skill_ids.push_back(effect.id);
    }
      
    for (int i = 0; i < Data::jewels().size(); ++i) {
      if (filter.Validate(i)) {
        const Jewel &jewel = Data::jewel(i);
        Signature key = Signature(jewel, skill_ids, effects, &valid);
        if (valid) {
          jewel_keys_[jewel.slots].push_back(key);
          jewel_ids_[jewel.slots].push_back(i);
        }
      }
    }
  }

  JewelPlan Solve(Signature key, int multiplier) const {
    Signature target = sig::InverseKey(key);
    int i(0), j(0), k(0);
    sig::KeySlots(key, &i, &j, &k);
    std::vector<int> ids;
    std::vector<int> body_ids;
      
    if (multiplier > 1) {
      int a(0), b(0), c(0);
      key.BodySlots(&a, &b, &c);
      std::vector<SearchCriteria> criterias;
      criterias.push_back({1, i, 1});
      criterias.push_back({2, j, 1});
      criterias.push_back({3, k, 1});
      criterias.push_back({1, a, multiplier});
      criterias.push_back({2, b, multiplier});
      criterias.push_back({3, c, multiplier});
      CHECK(Search(criterias,
                   5,  // top
                   0,  // criteria_id
                   0,  // jewel_id
                   target, &ids, &body_ids));
    } else {
      CHECK(Search({{1, i, 1}, {2, j, 1}, {3, k, 1}},
                   2,  // top
                   0,  // criteria_id
                   0,  // jewel_id
                   target, &ids, &body_ids));
    }
      
    JewelPlan result;
    for (int id : ids) {
      auto it = result.first.find(id);
      if (result.first.end() != it) {
        it->second++;
      } else {
        result.first[id] = 1;
      }
    }
    for (int id : body_ids) {
      auto it = result.second.find(id);
      if (result.second.end() != it) {
        it->second++;
      } else {
        result.second[id] = 1;
      }
    }
    return result;
  }

 private:
  struct SearchCriteria {
    int slots;
    int num;
    int multiplier;
  };
    
  bool Search(const std::vector<SearchCriteria> &targets,
              int top, int criteria_id, int jewel_id, 
              Signature key, 
              std::vector<int> *ids, 
              std::vector<int> *body_ids) const {
    if (-1 == top) return key.IsZero();

    if (criteria_id >= targets[top].num) {
      return Search(targets, top - 1, 0, 0, key, ids, body_ids);
    }

    const int &slots = targets[top].slots;
    const int &multiplier = targets[top].multiplier;
    for (int seq = jewel_id; seq < jewel_ids_[slots].size(); ++seq) {
      Signature jewel_key = jewel_keys_[slots][seq];
      if (multiplier > 1) {
        body_ids->push_back(jewel_ids_[slots][seq]);
        jewel_key *= multiplier;
      } else {
        ids->push_back(jewel_ids_[slots][seq]);
      }
      if (Search(targets, top, criteria_id + 1, seq,
                 key | jewel_key, ids, body_ids)) {
        return true;
      }
      if (multiplier > 1) {
        body_ids->pop_back();
      } else {
        ids->pop_back();
      }
    }
    return false;
  }
    
  std::array<std::vector<Signature>, 4> jewel_keys_;
  std::array<std::vector<int>, 4> jewel_ids_;
};

}

#endif  // _MONSTER_AVENGERS_UTILS_JEWELS_QUERY_
