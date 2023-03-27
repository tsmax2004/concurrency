#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/spin/wait.hpp>

namespace exe::threads {

// Test-and-TAS spinlock

class SpinLock {
 public:
  void Lock() {
    while (locked_.exchange(true)) {
      while (locked_.load()) {
        twist::ed::CpuRelax();
      }
    }
  }

  void Unlock() {
    locked_.store(false);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::ed::stdlike::atomic<bool> locked_{false};
};

}  // namespace exe::threads
