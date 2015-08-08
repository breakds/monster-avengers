#ifndef _MONSTER_AVENGERS_CORE_ITERATORS_BASE_
#define _MONSTER_AVENGERS_CORE_ITERATORS_BASE_

#include <cstddef>
#include <vector>

namespace monster_avengers {

class BaseIterator {
 public:
  virtual bool Empty() const = 0;
  virtual bool Next() = 0;
  virtual void Reset() = 0;
};

template <typename OutputType>
class Iterator : public BaseIterator {
 public:
  virtual const OutputType &Get() const = 0;
};

template <typename OutputType>
class ListIterator : public Iterator<OutputType> {
 public:
  explicit ListIterator(const std::vector<OutputType> &&input)
      : list_(input), pointer_(-1) {}

  inline bool Empty() const override {
    return pointer_ >= list_.size();
  }

  inline bool Next() override {
    pointer_++;
    return !Empty();
  }

  inline void Reset() override {
    pointer_ = -1;
  }

  inline const OutputType &Get() const override {
    return list_[pointer_];
  }

 private:
  const std::vector<OutputType> list_;
  size_t pointer_;
};

template <typename OutputType>
inline Iterator<OutputType> *CastIterator(BaseIterator *iterator) {
  return static_cast<Iterator<OutputType>*>(iterator);
}

}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_CORE_ITERATORS_BASE_
