#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdlib>

namespace exe::threads {

class WaitGroup {
 public:
  void Add(size_t num) {
    counter_.fetch_add(num);
  }

  void Done() {
    auto key = twist::ed::PrepareWake(counter_);
    if (counter_.fetch_sub(1) == 1) {
      twist::ed::WakeAll(key);
    }
  }

  void Wait() {
    auto old = counter_.load();
    while (old > 0) {
      twist::ed::Wait(counter_, old);
      old = counter_.load();
    }
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> counter_{0};
};

}  // namespace exe::threads
