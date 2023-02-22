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
    while (locked_.exchange(State::Locked) == State::Locked) {
      contention_.fetch_add(1);
      twist::ed::Wait(locked_, State::Locked);
      contention_.fetch_sub(1);
    }
  }

  void Unlock() {
    locked_.store(State::Free);
    if (contention_.load() >= 1) {
      twist::ed::WakeOne(twist::ed::PrepareWake(locked_));
    }
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> locked_{0};
  twist::ed::stdlike::atomic<uint32_t> contention_{0};
};

}  // namespace stdlike
