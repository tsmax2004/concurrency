#pragma once
#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/handle.hpp>
#include <exe/fibers/core/awaiter.hpp>

#include <exe/fibers/sched/suspend.hpp>

#include <exe/threads/spinlock.hpp>

#include <wheels/intrusive/list.hpp>

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::fibers {

// One-shot

class Event {
  using Fiber = exe::fibers::Fiber;
  using IntrusiveList = wheels::IntrusiveList<Fiber>;

 public:
  void Wait() {
    if (is_fired_.load()) {
      return;
    }

    fibers::Suspend(&event_awaiter_);
  }

  void Fire() {
    IntrusiveList tmp_list;
    {
      std::lock_guard guard(spinlock_);
      is_fired_.store(true);
      if (waiting_fibers_.IsEmpty()) {
        return;
      }
      tmp_list.Append(waiting_fibers_);
    }  // destroy guard

    while (tmp_list.HasItems()) {
      auto fiber = tmp_list.PopFront();
      fiber->Schedule();
    }
  }

 private:
  struct EventAwaiter : IAwaiter {
   public:
    explicit EventAwaiter(Event* event)
        : event_(event) {
    }

    void Await(FiberHandle fiber) override {
      std::unique_lock guard(event_->spinlock_);
      if (event_->is_fired_.load()) {
        guard.unlock();
        fiber.Switch();
        return;
      }

      event_->waiting_fibers_.PushBack(Fiber::Self());
    }

   private:
    Event* event_;
  };

  twist::ed::stdlike::atomic<bool> is_fired_{false};
  threads::SpinLock spinlock_;
  IntrusiveList waiting_fibers_;
  EventAwaiter event_awaiter_{this};
};

}  // namespace exe::fibers
