#ifndef _MONSTER_AVENGERS_CORE_ARMOR_UP_
#define _MONSTER_AVENGERS_CORE_ARMOR_UP_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "core/or_and_tree.h"
#include "core/pruners.h"
#include "iterators/base.h"
#include "supp/timer.h"
#include "utils/query.h"
#include "utils/signature.h"
#include "utils/jewels_query.h"



#define DEBUG_VERBOSE 0

namespace monster_avengers {

using dataset::Arsenal;
using dataset::Data;

using dataset::Armor;

const int FOUNDATION_NUM = 2;

class ArmorUp {
 public:
  ArmorUp() 
      : arsenal_(), pool_(),
        iterators_() {}
    
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

  void SearchCore(const Query &query) {
    // Add in custom armors
    InitializeArsenal(query);

    // Core Search
    CHECK_SUCCESS(ApplyFoundation(query));
    int foundations = (query.effects.size() < FOUNDATION_NUM)
        ? query.effects.size() : FOUNDATION_NUM;
    for (int i = 0; i < foundations; ++i) {
      CHECK_SUCCESS(ApplySingleJewelFilter(query.effects, i,
                                           query.jewel_filter));
    }
    for (int i = foundations; i < query.effects.size(); ++i) {
      CHECK_SUCCESS(ApplySkillSplitter(query, i));	
    }
    CHECK_SUCCESS(PrepareOutput());
    CHECK_SUCCESS(ApplyDefenseFilter(query));
    CHECK_SUCCESS(ApplyFinalizeFilter(query));
  }

  std::vector<ArmorSet> Search(const Query &query) {
    // Optimize the Query
    Query optimized_query = OptimizeQuery(query);

    SearchCore(optimized_query);

    std::vector<ArmorSet> result;
    auto *output = CastIterator<ArmorSet>(iterators_.back().get());
    while (output->Next()) {
      result.push_back(output->Get());
      if (result.size() >= query.max_results) break;
    }
    return result;
  }

  // TODO(breakds): Should not expose arsenal in this way.
  inline const Arsenal &GetArsenal() {
    return arsenal_;
  }

  // std::string SearchEncoded(const Query &query) {
  //   // Optimize the Query
  //   Query optimized_query = OptimizeQuery(query);

  //   SearchCore(optimized_query);

  //   // Prepare formatter
  //   EncodeFormatter formatter(&data_, optimized_query);

  //   std::string output;
  //   int count = 0;
  //   while (count < query.max_results && !armor_set_iterators_.back()->empty()) {
  //     formatter(**armor_set_iterators_.back(), &output);
  //     ++count;
  //     ++(*armor_set_iterators_.back());
  //   }
  //   return output;
  // }

  // std::wstring SearchSerialized(const Query &query) {
  //   // Optimize the Query
  //   Query optimized_query = OptimizeQuery(query);

  //   SearchCore(optimized_query);

  //   // Prepare formatter
  //   ResultSerializer serializer(&data_, optimized_query);

  //   std::string output;
  //   int count = 0;
  //   while (count < query.max_results && !armor_set_iterators_.back()->empty()) {
  //     serializer.Add(**armor_set_iterators_.back());
  //     ++count;
  //     ++(*armor_set_iterators_.back());
  //   }
  //   return serializer.ToString();
  // }

  // void Explore(const Query &input_query,
  //              const std::string output_path = "") {
  //   Timer overall_timer;
  //   overall_timer.Tic();
  //   Timer timer;
  //   pool_.PushSnapshot();

  //   ExploreFormatter formatter(output_path);
      
  //   for (int i = 1; i < data_.skill_systems().size(); ++i) {
  //     timer.Tic();
  //     if (input_query.HasSkill(i)) {
  //       formatter.Push(i, false,
  //                      data_.skill_system(i).name,
  //                      timer.Toc());
  //       continue;
  //     }
  //     pool_.RestoreSnapshot();
  //     iterators_.clear();
        
  //     Query updated_query = input_query;
  //     updated_query.effects.push_back({
  //         i, data_.skill_system(i).LowestPositivePoints()});
  //     Query query = OptimizeQuery(updated_query, false);
        
  //     // Add in custom armors
  //     InitializeExtraArmors(query);

  //     // Core Search
  //     CHECK_SUCCESS(ApplyFoundation(query));
  //     for (int i = 0; i < FOUNDATION_NUM; ++i) {
  //       CHECK_SUCCESS(ApplySingleJewelFilter(query.effects, i, 
  //     				       query.jewel_filter));
  //     }
  //     for (int i = FOUNDATION_NUM; i < query.effects.size(); ++i) {
  //       CHECK_SUCCESS(ApplySkillSplitter(query, i));	
  //     }

  //     formatter.Push(i, !iterators_.back()->empty(), 
  //                    data_.skill_system(i).name,
  //                    timer.Toc());
  //   }
  //   wprintf(L"Overall: %.4lf sec\n", overall_timer.Toc());
  // }

  Query OptimizeQuery(const Query &query) {
    std::vector<double> scores;
    std::vector<int> indices;
    for (int i = 0; i < query.effects.size(); ++i) {
      const Effect &effect = query.effects[i];
      indices.push_back(i);
      scores.push_back(Data::EffectScore(effect));
#if DEBUG_VERBOSE > 0
      wprintf(L"(%03d) %.05lf\n", effect.id, scores.back());
#endif
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

  // ----- For Debugging -----
  void Summarize() {
    Log(INFO, L"OR Nodes: %lld\n", pool_.OrSize());
    Log(INFO, L"AND Nodes: %lld\n", pool_.AndSize());
  } 

 private:

  void InitializeArsenal(const Query &query) {
    arsenal_.clear();
    // Amulets
    for (const Armor &amulet : query.amulets) {
      arsenal_.AddArmor(amulet);
    }
  }
    
  // Returns a vector of newly created or nodes' indices.
  std::vector<int> ClassifyArmors(ArmorPart part,
                                  const Query &query) {
    std::unordered_map<Signature, std::vector<int> > armor_map;

    std::vector<Effect> effects;
    int query_size = query.effects.size();
    for (int i = 0; i < (std::min)(query_size, FOUNDATION_NUM); ++i) {
      effects.push_back(query.effects[i]);
    }

#if DEBUG_VERBOSE > 1
    wprintf(L"---------- Classify %s ----------\n",
            StringifyEnum(part).c_str());
#endif
    // TODO(breakds): The for loop here needs optimization.
    for (int id = 0; id < arsenal_.size(); ++id) {
      const Armor &armor = arsenal_[id];
      if (armor.part == part &&
          query.armor_filter.Validate(armor, id)) {
#if DEBUG_VERBOSE > 1
        arsenal_.PrintArmor(id, 0);
#endif
        Signature key(armor, effects);
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
    iterators_.emplace_back(new ListIterator<TreeRoot>(
        Foundation(query)));
    return Status(SUCCESS);
  }

  // TODO(breakds): Change the name here.
  Status ApplySingleJewelFilter(const std::vector<Effect> &effects, 
                                int effect_id, 
                                const JewelFilter &filter) {
    iterators_.emplace_back(new JewelPruner(
        CastIterator<TreeRoot>(iterators_.back().get()),
        &pool_, effect_id, effects, filter));
    return Status(SUCCESS);
  }

  Status ApplySkillSplitter(const Query &query,
                            int effect_id) {
    iterators_.emplace_back(new SkillSplitPruner(
        CastIterator<TreeRoot>(iterators_.back().get()),
        arsenal_, &pool_, effect_id, query));
    return Status(SUCCESS);
  }
  
  Status PrepareOutput() {
    iterators_.emplace_back(
        new Unpacker(CastIterator<TreeRoot>(iterators_.back().get()),
                     &pool_));
    return Status(SUCCESS);
  }

  Status ApplyDefenseFilter(const Query &query) {
    iterators_.emplace_back(new DefensePruner(
        CastIterator<RawArmorSet>(iterators_.back().get()),
        &arsenal_, query.defense));
    return Status(SUCCESS);
  }

  Status ApplyFinalizeFilter(const Query &query) {
    iterators_.emplace_back(new Finalizer(
        CastIterator<RawArmorSet>(iterators_.back().get()),
        query, &arsenal_, 1));
    return Status(SUCCESS);
  }
    
  Arsenal arsenal_;
  NodePool pool_;
  std::vector<std::unique_ptr<BaseIterator> > iterators_;
};
}

#endif  // _MONSTER_AVENGERS_CORE_ARMOR_UP_

