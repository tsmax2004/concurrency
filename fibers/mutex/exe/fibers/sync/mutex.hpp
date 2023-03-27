#pragma once

// std::lock_guard and std::unique_lock
#include <mutex>

#include <exe/fibers/core/awaiter.hpp>

#include <exe/fibers/sched/suspend.hpp>
#include <exe/fibers/sched/yield.hpp>

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::fibers {

// Use strand-like algorithm

class Mutex {
 public:
  void Lock() {
    MutexAwaiter awaiter(*this);
    Suspend(awaiter);
  }

  void Unlock() {
    Yield();
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
    friend Mutex;

   public:
    explicit MutexAwaiter(Mutex& mutex)
        : mutex_(mutex) {
    }

    bool AwaitReady() override {
      return false;
    }

    bool AwaitSuspend(FiberHandle fiber) override {
      fiber_ = fiber;

      auto waiter_num = mutex_.waiters_cnt_.fetch_add(1);
      mutex_.Push(this);

      if (waiter_num == 0) {
        mutex_.RunCriticalSection();
      }

      return true;
    }

    void AwaitResume() override {
      fiber_.Switch();
    }

   private:
    Mutex& mutex_;
    FiberHandle fiber_;
    MutexAwaiter* next_;
  };

  void Push(MutexAwaiter* waiter) {
    auto* old = waiters_.load();
    do {
      waiter->next_ = old;
    } while (!waiters_.compare_exchange_strong(old, waiter));
  }

  void RunCriticalSection() {
    auto* batch = waiters_.exchange(nullptr);
    size_t cnt = 0;
    while (batch != nullptr) {
      auto* next = batch->next_;
      batch->AwaitResume();
      batch = next;
      ++cnt;
    }

    if (waiters_cnt_.fetch_sub(cnt) != cnt) {
      RunCriticalSection();
    }
  }

  twist::ed::stdlike::atomic<size_t> waiters_cnt_{0};
  twist::ed::stdlike::atomic<MutexAwaiter*> waiters_{nullptr};
};

}  // namespace exe::fibers
