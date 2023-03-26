#pragma once

// std::lock_guard and std::unique_lock
#include <mutex>

#include <exe/fibers/core/fiber.hpp>

#include <exe/fibers/sched/suspend.hpp>

#include <exe/threads/spinlock.hpp>

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::fibers {

class Mutex {
  using Fiber = exe::fibers::Fiber;
  using IntrusiveList = wheels::IntrusiveList<Fiber>;

 public:
  void Lock() {
    fibers::Suspend(&awaiter_);
  }

  void Unlock() {
    std::lock_guard guard(spin_lock_);

    if (waiting_fibers_.IsEmpty()) {
      is_locked_ = false;
      return;
    }
    waiting_fibers_.PopFront()->Schedule();
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  struct MutexAwaiter : IAwaiter {
   public:
    MutexAwaiter(Mutex* mutex)
        : mutex_(mutex) {
    }

    void Await(FiberHandle fiber) override {
      std::unique_lock guard(mutex_->spin_lock_);

      if (!mutex_->is_locked_) {
        mutex_->is_locked_ = true;
        guard.unlock();
        fiber.Switch();
        return;
      }

      mutex_->waiting_fibers_.PushBack(Fiber::Self());
    }

   private:
    Mutex* mutex_;
  };

  twist::ed::stdlike::atomic<bool> is_locked_{false};
  threads::SpinLock spin_lock_;
  IntrusiveList waiting_fibers_;
  MutexAwaiter awaiter_{this};
};

}  // namespace exe::fibers
