#include <unordered_map>
#include "supp/helpers.h"
#include "tiny_map.h"

using namespace monster_avengers;

inline int FakeKey(int value) {
  return ((value * value) % 13) - 7;
}

bool IntSetEqual(const std::vector<int> &a, const std::vector<int> &b) {
  if (a.size() != b.size()) return false;

  std::unordered_map<int, int> counter;
  for (int x : a) {
    auto iter = counter.find(x);
    if (counter.end() == iter) {
      counter[x] = 1;
    } else {
      iter->second++;
    }
  }

  for (int x : b) {
    auto iter = counter.find(x);
    if (counter.end() == iter) {
      return false;
    } else {
      iter->second--;
    }
  }

  for (const auto& item : counter) {
    if (0 != item.second) return false;
  }
  return true;
}

template <typename TinyMap>
bool Verify(const std::unordered_map<int, std::vector<int> > &golden,
            const TinyMap &result) {
  if (golden.size() != result.size()) return false;
  for (const auto &item : result) {
    int key = item.first;
    wprintf(L"key %d:", key);
    for (const auto &element : item.second) {
      wprintf(L" %d", element);
    }
    wprintf(L"\n");
    
    auto iter = golden.find(key);
    if (golden.end() == iter) return false;
    if (!IntSetEqual(iter->second, item.second)) return false;
  }
  return true;
}

template <typename TinyMap>
void TestMap() {
  TinyMap tiny_map;
  std::unordered_map<int, std::vector<int> > golden;
  
  for (int i = -100; i < 100; ++i) {
    int key = FakeKey(i);

    // Golden
    auto iter = golden.find(key);
    if (golden.end() == iter) {
      golden[key] = {i};
    } else {
      iter->second.push_back(i);
    }

    // Tiny Map
    tiny_map.Push(key, i);
  }

  CHECK(Verify(golden, tiny_map));
}

int main(){
  TestMap<VectorBasedListMap>();
  TestMap<MapBasedListMap>();
  TestMap<CompositeListMap>();
  Log(OK, L"Test tiny_map_test completed.");
  return 0;
}
