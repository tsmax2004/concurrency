#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdint>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    counter_.fetch_add(1);
    while (locked_.exchange(State::Contention) != State::Free) {
      twist::ed::Wait(locked_, State::Contention);
    }
    locked_.store(State::Locked);
  }

  void Unlock() {
    counter_.fetch_sub(1);
    auto key = twist::ed::PrepareWake(locked_);

    if (counter_ >= 1) {
      locked_.store(State::Contention);
    }
    auto old = locked_.exchange(State::Free);

    if (old == State::Contention) {
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

  twist::ed::stdlike::atomic<uint32_t> locked_{0};
  twist::ed::stdlike::atomic<uint32_t> counter_{0};
};

}  // namespace stdlike
