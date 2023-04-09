#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/threads/support/spin_wait.hpp>

////////////////////////////////////////////////////////////////////////////////

namespace exe::threads {

// Test-Test-and-Set (TTAS) spinlock
class SpinLock {
 public:
  void Lock() {
    support::SpinWait spin_wait;
    while (!is_free_.exchange(false, std::memory_order_acquire)) {
      while (!is_free_.load(std::memory_order_relaxed)) {
        spin_wait();
      }
    }
  }

  bool TryLock() {
    return is_free_.exchange(false, std::memory_order_acquire);
  }

  void Unlock() {
    is_free_.store(true, std::memory_order_release);
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
  twist::ed::stdlike::atomic<bool> is_free_{true};
};

}  // namespace exe::threads