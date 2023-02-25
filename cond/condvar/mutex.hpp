#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdint>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    contention_.fetch_add(1);
    while (locked_.exchange(State::Locked) == State::Locked) {
      twist::ed::Wait(locked_, State::Locked);
    }
  }

  void Unlock() {
    contention_.fetch_sub(1);
    auto key = twist::ed::PrepareWake(locked_);

    locked_.store(State::Free);
    if (contention_.load() >= 1) {
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
  };

  twist::ed::stdlike::atomic<uint32_t> locked_{0};
  twist::ed::stdlike::atomic<uint32_t> contention_{0};
};

}  // namespace stdlike
