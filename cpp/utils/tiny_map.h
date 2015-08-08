#ifndef _MONSTER_AVENGERS_UTILS_TINY_MAP_
#define _MONSTER_AVENGERS_UTILS_TINY_MAP_

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <vector>

namespace monster_avengers {

class VectorBasedListMap {
 public:
  typedef std::vector<std::pair<int, std::vector<int> > > Container;
  VectorBasedListMap() : container_() {}

  void Push(int key, int value) {
    for (auto &item : container_) {
      if (item.first == key) {
        item.second.push_back(value);
        return;
      }
    }
    container_.emplace_back();
    container_.back().first = key;
    container_.back().second.push_back(value);
  }

  inline Container::iterator begin() {
    return container_.begin();
  }

  inline Container::iterator end() {
    return container_.end();
  }

  inline Container::const_iterator begin() const {
    return container_.begin();
  }

  inline Container::const_iterator end() const {
    return container_.end();
  }


  inline size_t size() const {
    return container_.size();
  }

 private:
  Container container_;
};


class MapBasedListMap {
 public:
  typedef std::unordered_map<int, std::vector<int> > Container;
  MapBasedListMap() : container_() {}

  void Push(int key, int value) {
    auto iter = container_.find(key);
    if (container_.end() == iter) {
      container_[key] = {value};
    } else {
      iter->second.push_back(value);
    }
  }

  inline Container::iterator begin() {
    return container_.begin();
  }

  inline Container::iterator end() {
    return container_.end();
  }

  inline Container::const_iterator begin() const {
    return container_.begin();
  }

  inline Container::const_iterator end() const {
    return container_.end();
  }


  inline size_t size() const {
    return container_.size();
  }

 private:
  Container container_;
};


class CompositeListMap {
 public:
  typedef std::vector<std::pair<int, std::vector<int> > > Container;
  typedef std::array<unsigned char, 64> Map;

  CompositeListMap() : container_(), index_map_() {
    memset(&index_map_[0], 255, 64);
  }
      
  void Push(int key, int value) {
    int internal_key = key + 25;
    if (255 != index_map_[internal_key]) {
      container_[index_map_[internal_key]].second.push_back(value);
      return;
    }
    index_map_[internal_key] = static_cast<unsigned char>(container_.size());
    container_.emplace_back();
    container_.back().first = key;
    container_.back().second.push_back(value);
  }

  inline Container::iterator begin() {
    return container_.begin();
  }

  inline Container::iterator end() {
    return container_.end();
  }

  inline Container::const_iterator begin() const {
    return container_.begin();
  }

  inline Container::const_iterator end() const {
    return container_.end();
  }

  inline size_t size() const {
    return container_.size();
  }

 private:
  Container container_;
  Map index_map_;
};

}  // monster_avengers

#endif  // _MONSTER_AVENGERS_UTILS_TINY_MAP_
