#pragma once

#include <cstdlib>

#include <exe/fibers/core/fiber.hpp>

#include <exe/fibers/sched/suspend.hpp>

#include <exe/threads/spinlock.hpp>

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::fibers {

// https://gobyexample.com/waitgroups

class WaitGroup {
  using Fiber = exe::fibers::Fiber;
  using IntrusiveList = wheels::IntrusiveList<Fiber>;

 public:
  void Add(size_t count) {
    counter_.fetch_add(count);
  }

  void Done() {
    IntrusiveList tmp_list;
    {
      std::lock_guard guard(spin_lock_);
      if (counter_.fetch_sub(1) != 1) {
        return;
      }
      tmp_list.Append(waiting_fibers_);
    }  // destroy guard

    while (tmp_list.HasItems()) {
      auto fiber = tmp_list.PopFront();
      fiber->Schedule();
    }
  }

  void Wait() {
    if (counter_.load() == 0) {
      return;
    }

    fibers::Suspend(&awaiter_);
  }

 private:
  struct WaitGroupAwaiter : IAwaiter {
   public:
    explicit WaitGroupAwaiter(WaitGroup* wg)
        : wg_(wg) {
    }

    void Await(FiberHandle fiber) override {
      std::unique_lock guard(wg_->spin_lock_);

      if (wg_->counter_.load() == 0) {
        guard.unlock();
        fiber.Switch();
        return;
      }

      wg_->waiting_fibers_.PushBack(Fiber::Self());
    }

   private:
    WaitGroup* wg_;
  };

  twist::ed::stdlike::atomic<size_t> counter_{0};
  threads::SpinLock spin_lock_;
  IntrusiveList waiting_fibers_;
  WaitGroupAwaiter awaiter_{this};
};

}  // namespace exe::fibers
