#pragma once

// std::lock_guard and std::unique_lock
#include <mutex>

#include <exe/fibers/core/awaiter.hpp>

#include <exe/fibers/sched/suspend.hpp>
#include <exe/fibers/sched/yield.hpp>

#include <exe/fibers/sync/mutex_waiting_queue.hpp>

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::fibers {

// Use strand-like algorithm

class Mutex {
 public:
  void Lock() {
    MutexLockAwaiter awaiter(*this);
    Suspend(awaiter);
  }

  void Unlock() {
    MutexUnlockAwaiter awaiter(*this);
    Suspend(awaiter);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  struct MutexLockAwaiter : IAwaiter,
                            IntrusiveNode<MutexLockAwaiter> {
    friend Mutex;

   public:
    explicit MutexLockAwaiter(Mutex& mutex)
        : mutex_(mutex) {
    }

    bool AwaitSuspend(FiberHandle fiber) override {
      fiber_ = fiber;
      return !mutex_.waiting_queue_.TryLockLock(this);
    }

   private:
    Mutex& mutex_;
    FiberHandle fiber_;
  };

  struct MutexUnlockAwaiter : IAwaiter {
    friend Mutex;

    explicit MutexUnlockAwaiter(Mutex& mutex)
        : mutex_(mutex) {
    }

    bool AwaitSuspend(FiberHandle fiber) override {
      auto* next = mutex_.waiting_queue_.Pop();
      if (next == nullptr) {
        return false;
      } else {
        fiber.Schedule();
        next->fiber_.Switch();
        return true;
      }
    }

   private:
    Mutex& mutex_;
    FiberHandle fiber_;
  };

  WaitingQueue<MutexLockAwaiter> waiting_queue_;
};

}  // namespace exe::fibers
