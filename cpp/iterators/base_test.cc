#include <string>
#include <vector>

#include "base.h"
#include "supp/helpers.h"

using namespace monster_avengers;

class OddFilter : public Iterator<int> {
 public:
  OddFilter(Iterator<int> *input)
      : base_iter_(input) {}


  inline bool Empty() const override {
    return base_iter_->Empty();
  }

  inline bool Next() override {
    while (base_iter_->Next()) {
      if (1 == (base_iter_->Get() % 2)) {
        break;
      }
    }
    return !Empty();
  }

  inline void Reset() override {
    base_iter_->Reset();
  }

  inline const int &Get() const override {
    return base_iter_->Get();
  }
  
 private:
  Iterator<int> *base_iter_;
};

class Splitter : public Iterator<std::string> {
 public:
  Splitter(Iterator<int> *input)
      : base_iter_(input), stack_() {}

  inline bool Empty() const override {
    return base_iter_->Empty() && stack_.empty();
  }

  inline bool Next() override {
    if (!stack_.empty()) {
      stack_.pop_back();
    }

    while (stack_.empty()) {
      if (base_iter_->Next()) {
        for (int i = 1; i < base_iter_->Get(); ++i) {
          stack_.push_back(std::to_string(i) + "+" +
                           std::to_string(base_iter_->Get() - i));
        }
      } else {
        break;
      }
    }
    return !Empty();
  }

  inline const std::string &Get() const override {
    return stack_.back();
  }

  inline void Reset() override {
    base_iter_->Reset();
    stack_.clear();
  }

 private:
  Iterator<int> *base_iter_;
  std::vector<std::string> stack_;
};

int main() {
  ListIterator<int> first({0, 1, 2, 3, 4, 5, 6, 7, 8});

  int i = 0;
  while (first.Next()) {
    CHECK(i == first.Get());
    ++i;
  }
  CHECK(i == 9);

  first.Reset();
  OddFilter second(&first);

  i = 1;
  while (second.Next()) {
    CHECK(i == second.Get());
    i += 2;
  }
  CHECK(i == 9);

  second.Reset();

  i = 1;
  while (second.Next()) {
    CHECK(i == second.Get());
    i = i + 2;
  }
  CHECK(i == 9);

  Splitter third(&second);

  third.Reset();

  {
    std::vector<std::string> expected = {
      "2+1", "1+2", "4+1", "3+2", "2+3", "1+4",
      "6+1", "5+2", "4+3", "3+4", "2+5", "1+6"};
    int i = 0;
    while (third.Next()) {
      CHECK(expected[i++] == third.Get());
    }
    CHECK(i == 12);
  }
  
  Log(OK, L"Test iterator/base_test completed.");
  return 0;
}
