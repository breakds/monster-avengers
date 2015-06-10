#ifndef _MONSTER_AVENGERS_DATASET_REINDEXED_TABLE_H_
#define _MONSTER_AVENGERS_DATASET_REINDEXED_TABLE_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "supp/helpers.h"

namespace monster_avengers {

namespace dataset {

class IndexMap {
 public:
  IndexMap() = default;

  inline int Update(int external_id, int internal_id) {
    // Append Mode
    if (internal_id == forward_map_.size()) {
      forward_map_.push_back(external_id);
    } else if (internal_id < forward_map_.size()) {
      forward_map_[internal_id] = external_id;
    } else {
      Log(WARNING, L"IndexMap::Update resized forward map.");
      forward_map_.resize(internal_id + 1, -1);
      forward_map_[internal_id] = external_id;
    }
    backward_map_[external_id] = internal_id;
    return internal_id;
  }

  inline int Internalize(int external_id) const {
    return backward_map_.at(external_id);
  }

  inline int Externalize(int internal_id) const {
    return forward_map_[internal_id];
  }

 private:
  std::vector<int> forward_map_; // internal_id -> external_id
  std::unordered_map<int, int> backward_map_; // external_id -> internal_id
};

template <typename ElementType>
class ReindexedTable {
 public:
  ReindexedTable() = default;

  inline void ResetIndexMap(IndexMap *index_map) {
    index_map_.reset(index_map);
  }

  inline void ResetIndexMap() {
    index_map_.reset(new IndexMap);
  }

  template <typename T>
  inline void ResetIndexMap(ReindexedTable<T> *other) {
    index_map_ = other->GetIndexMap();
  }

  inline std::shared_ptr<IndexMap> &GetIndexMap() {
    return index_map_;
  }

  // Will update index_map_.
  inline void Add(const ElementType &element, int external_id) {
    CHECK(index_map_);
    elements_.push_back(element);
    index_map_->Update(external_id, elements_.size() - 1);
  }

  // Will not affect index_map_.
  inline void Update(const ElementType &element, int external_id) {
    CHECK(index_map_);
    int id = index_map_->Internalize(external_id);
    if (id >= elements_.size()) {
      elements_.resize(id + 1);
    }
    elements_[id] = element;
  }

  inline ElementType &operator[](int internal_id) {
    return elements_[internal_id];
  }

 private:
  std::shared_ptr<IndexMap> index_map_;
  std::vector<ElementType> elements_;
};


}  // namespace dataset

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_DATASET_REINDEXED_TABLE_H_
