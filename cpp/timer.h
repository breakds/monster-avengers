#ifndef _MONSTER_AVENGERS_TIMER_
#define _MONSTER_AVENGERS_TIMER_

#include <chrono>

class Timer {
public:
  Timer() = default;
  
  void Tic() {
    start_ = std::chrono::system_clock::now();
  }

  double Toc() {
    std::chrono::time_point<std::chrono::system_clock> end = 
      std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start_;
    return elapsed.count();
  }
  
private:
  std::chrono::time_point<std::chrono::system_clock> start_;
};

#endif  // _MONSTER_AVENGERS_TIMER_

