#ifndef _MONSTER_AVENGERS_JEWELS_QUERY_
#define _MONSTER_AVENGERS_JEWELS_QUERY_

#include <array>
#include <vector>
#include <unordered_set>
#include "signature.h"

namespace monster_avengers {

  
  class HoleClient {
  public:
    constexpr static int MAX_ONES = 24;
    constexpr static int MAX_TWOS = 8;
    constexpr static int MAX_THREES = 8;

    HoleClient(const DataSet &data, 
               const std::vector<int> &skill_ids,
               const std::vector<Effect> &effects)
      : jewel_keys_() {
      bool valid = false;
      for (Jewel jewel : data.Jewels()) {
        Signature key = sig::JewelKey(jewel, skill_ids, 
                                      effects, &valid);
        if (valid) {
          jewel_keys_[jewel.holes].insert(key);
        }
      }
      
      buffer_[0].insert(0);
      fixed_buffer_[1][0].insert(0);
      fixed_buffer_[2][0].insert(0);
      fixed_buffer_[3][0].insert(0);
    }
    
    HoleClient(const DataSet &data, 
               const std::vector<Effect> &effects) 
      : HoleClient(data, SkillIdsFromEffects(effects), effects) {}
    
    HoleClient(const DataSet &data, int skill_id, 
               const std::vector<Effect> &effects) 
      : HoleClient(data, std::vector<int>({skill_id}), 
                   effects) {}
    
    inline const std::unordered_set<Signature> &Query(Signature input) {
      int i(0), j(0), k(0);
      sig::KeyHoles(input, &i, &j, &k);
      return Calculate(i, j, k);
    }

    inline const std::unordered_set<Signature> &Query(int i, 
                                                      int j, 
                                                      int k) {
      return Calculate(i, j, k);
    }

    // Use the hole aligment from stuffed to stuff the original hole
    // aligment, and get the residual hole alignment.
    static void GetResidual(Signature original, Signature stuffed,
                            int *i, int *j, int *k) {
      sig::KeyHoles(original, i, j, k);
      int one(0), two(0), three(0);
      sig::KeyHoles(stuffed, &one, &two, &three);
      
      // Handle 3 holes
      *k -= three;

      // Handle 2 holes
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
          *k -= ((one + 2) / 3);
          int remain = one % 3;
          if (1 == remain) {
            (*j)++;
          } else if (2 == remain) {
            (*i)++;
          }
        }
      }
    }

    // This is only for unit test purpose.
    std::unordered_set<Signature> DFS(int i, int j, int k) {
      std::array<std::vector<Signature>, 4> jewels;
      for (int holes = 1; holes <= 3; ++holes) {
        for (const Signature &key : jewel_keys_[holes]) {
          jewels[holes].push_back(key);
        }
      }
      std::unordered_set<Signature> result;
      DFS(i, j, k, 3, 0, 0, jewels, &result);
      return result;
    }

  private:
    std::vector<int> SkillIdsFromEffects(const std::vector<Effect> &effects) {
      std::vector<int> skill_ids;
      for (const Effect &effect : effects) {
        skill_ids.push_back(effect.skill_id);
      }
      return skill_ids;
    }

    inline void SetProduct(const std::unordered_set<Signature> &a,
                    const std::unordered_set<Signature> &b,
                    std::unordered_set<Signature> *c) {
      for (Signature key_a : a) {
        for (Signature key_b : b) {
          c->insert(sig::CombineKey(key_a, key_b));
        }
      }
    }

    inline void SetUnion(const std::unordered_set<Signature> &input,
                  std::unordered_set<Signature> *base) {
      for (Signature key : input) {
        base->insert(key);
      }
    }

    const std::unordered_set<Signature> &CalculateFixed(int holes, int i) {
      if (!fixed_buffer_[holes][i].empty()) {
        return fixed_buffer_[holes][i];
      }
      SetProduct(CalculateFixed(holes, i - 1),
                 jewel_keys_[holes],
                 &fixed_buffer_[holes][i]);
      return fixed_buffer_[holes][i];
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

    void DFS(int i, int j, int k, int holes, int id, Signature key,
             const std::array<std::vector<Signature>, 4> &jewels,
             std::unordered_set<Signature> *result) {
      result->insert(key);
      if (0 == i + j + k) {
        return;
      }
      int p = holes;
      int q = id;
      do {
        if (1 == p) {
          if (0 < i) {
            DFS(i - 1, j, k, p, q, 
                sig::CombineKey(key, jewels[p][q]),
                jewels, result);
          } else if (0 < j) {
            DFS(i + 1, j - 1, k, p, q,
                sig::CombineKey(key, jewels[p][q]),
                jewels, result);
          } else {
            DFS(i, j + 1, k - 1, p, q,
                sig::CombineKey(key, jewels[p][q]),
                jewels, result);
          }
        } else if (2 == p) {
          if (0 < j) {
            DFS(i, j - 1, k, p, q,
                sig::CombineKey(key, jewels[p][q]),
                jewels, result);
          } else if (0 < k) {
            DFS(i + 1, j, k - 1, p, q,
                sig::CombineKey(key, jewels[p][q]),
                jewels, result);
          }
        } else if (3 == p) {
          if (0 < k) {
            DFS(i, j, k - 1, p, q,
                sig::CombineKey(key, jewels[p][q]),
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
               MAX_ONES * MAX_TWOS * MAX_THREES> buffer_;
  };

}

#endif  // _MONSTER_AVENGERS_JEWELS_QUERY_
