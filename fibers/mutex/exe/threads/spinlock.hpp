#pragma once
#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/spin/wait.hpp>

namespace exe::threads {

// Test-and-TAS spinlock

class SpinLock {
 public:
  void Lock() {
    twist::ed::SpinWait spin_wait;
    while (is_locked_.exchange(true)) {
      while (is_locked_.load()) {
        spin_wait();
      }
    }
  }

  void Unlock() {
    is_locked_.store(false);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::ed::stdlike::atomic<bool> is_locked_{false};
};

}  // namespace exe::threads
