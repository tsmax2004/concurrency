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
      if (mutex_.worker_exists_) {
        fiber.Schedule();
        return true;
      }

      LockAwaiter* next = mutex_.waiting_queue_.Pop();
      if (next == nullptr) {
        return false;
      }

      Mutex& mutex = mutex_;
      fiber.Schedule();
      do {
        mutex.worker_exists_ = true;
        next->fiber_.Switch();
        mutex.worker_exists_ = false;
        next = mutex.waiting_queue_.Pop();
      } while (next != nullptr);

      return true;
    }

   private:
    Mutex& mutex_;
  };

  support::WaitingQueue<LockAwaiter> waiting_queue_;
  bool worker_exists_{false};
};

}  // namespace exe::fibers
