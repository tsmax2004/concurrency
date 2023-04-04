#pragma once

// std::lock_guard and std::unique_lock
#include <mutex>

#include <exe/fibers/sync/mutex_waiting_queue.hpp>

#include <exe/fibers/core/awaiter.hpp>

#include <exe/fibers/sched/suspend.hpp>

namespace exe::fibers {

class Mutex {
 public:
  void Lock() {
    LockAwaiter awaiter(*this);
    Suspend(awaiter);
  }

  void Unlock() {
    UnlockAwaiter awaiter(*this);
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
  struct LockAwaiter : IAwaiter,
                       support::IntrusiveQueueNode<LockAwaiter> {
    friend Mutex;

   public:
    explicit LockAwaiter(Mutex& mutex)
        : mutex_(mutex) {
    }

    bool AwaitSuspend(FiberHandle fiber) {
      fiber_ = fiber;
      if (mutex_.waiting_queue_.TryLockPush(this)) {
        mutex_.waiting_queue_.Pop();
        return false;
      }
      return true;
    }

   private:
    Mutex& mutex_;
    FiberHandle fiber_;
  };

  struct UnlockAwaiter : IAwaiter {
   public:
    explicit UnlockAwaiter(Mutex& mutex)
        : mutex_(mutex) {
    }

    bool AwaitSuspend(FiberHandle fiber) {
      fiber_ = fiber;
      if (mutex_.in_contention_) {
        mutex_.releasing_awaiter_ = this;
        return true;
      }

      Mutex& mutex = mutex_;
      auto releasing_awaiter = this;
      LockAwaiter* next;

      while ((next = mutex.waiting_queue_.Pop()) != nullptr) {
        releasing_awaiter->fiber_.Schedule();

        mutex.in_contention_ = true;
        next->fiber_.Switch();
        mutex.in_contention_ = false;

        releasing_awaiter = mutex.releasing_awaiter_;
      }

      releasing_awaiter->fiber_.Switch();
      return true;
    }

   private:
    Mutex& mutex_;
    FiberHandle fiber_;
  };

  support::WaitingQueue<LockAwaiter> waiting_queue_;
  bool in_contention_{false};
  UnlockAwaiter* releasing_awaiter_{nullptr};
};

}  // namespace exe::fibers
