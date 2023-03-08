#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdint>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    if (state_.exchange(State::Locked) == State::Free) {
      return;
    }

    while (state_.exchange(State::Contention) != State::Free) {
      twist::ed::Wait(state_, State::Contention);
    }
  }

  void Unlock() {
    auto key = twist::ed::PrepareWake(state_);
    if (state_.exchange(State::Free) == State::Contention) {
      twist::ed::WakeOne(key);
    }
  }

  // BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  enum State : uint32_t {
    Free = 0,
    Locked = 1,
    Contention = 2,
  };

  twist::ed::stdlike::atomic<uint32_t> state_{0};
};

}  // namespace stdlike
