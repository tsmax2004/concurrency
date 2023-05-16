#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

namespace exe::executors::tp::fast {

class TaskCounter {
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
    uint32_t old;
    while ((old = counter_.load()) != 0) {
      twist::ed::Wait(counter_, old);
    }
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> counter_{0};
};

}  // namespace exe::executors::tp::fast