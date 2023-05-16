#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdlib>

namespace exe::threads {

class Event {
 public:
  void Set() {
    auto key = twist::ed::PrepareWake(is_ready_);
    is_ready_.store(1);
    twist::ed::WakeOne(key);
  }

  void Wait() {
    twist::ed::Wait(is_ready_, 0);
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> is_ready_{0};
};

}  // namespace exe::threads
