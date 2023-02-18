#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
  enum State : uint32_t {
    Free = 0,
    Locked = 1,
  };

 public:
  void Lock() {
    auto old_locked = locked_.exchange(State::Locked);
    while (old_locked == State::Locked) {
      twist::ed::Wait(locked_, State::Locked);
      old_locked = locked_.exchange(State::Locked);
    }
  }

  void Unlock() {
    auto wake_key = twist::ed::PrepareWake(locked_);

    locked_.store(State::Free);
    twist::ed::WakeOne(wake_key);
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> locked_{0};
};

}  // namespace stdlike
