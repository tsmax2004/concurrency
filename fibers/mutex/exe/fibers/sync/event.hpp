#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/fibers/core/handle.hpp>
#include <exe/fibers/sched/suspend.hpp>

namespace exe::fibers {

// Source:
// https://lewissbaker.github.io/2017/11/17/understanding-operator-co-await

// One-shot

class Event {
 public:
  void Wait() {
    if (IsFired()) {
      return;
    }

    EventAwaiter awaiter(*this);
    Suspend(awaiter);
  }

  void Fire() {
    void* old_state = event_state_.exchange(this);

    auto* awaiter = static_cast<EventAwaiter*>(old_state);
    while (awaiter != nullptr) {
      auto* next = awaiter->next_;
      awaiter->fiber_.Schedule();
      awaiter = next;
    }
  }

 private:
  bool IsFired() {
    return event_state_.load() == this;
  }

  struct EventAwaiter : IAwaiter {
    friend Event;

   public:
    explicit EventAwaiter(Event& event)
        : event_(event) {
    }

    bool AwaitSuspend(FiberHandle fiber) override {
      fiber_ = fiber;

      void* old = event_.event_state_.load();

      do {
        if (event_.IsFired()) {
          return false;
        }

        next_ = static_cast<EventAwaiter*>(old);
      } while (!event_.event_state_.compare_exchange_strong(old, this));

      return true;
    }

   private:
    Event& event_;
    FiberHandle fiber_;
    EventAwaiter* next_{nullptr};  // in waiting queue
  };

  // State explanation:
  // 1) nullptr       - not fired (by default)
  // 2) this          - fired
  // 3) EventAwaiter* - ptr to waiting queue
  twist::ed::stdlike::atomic<void*> event_state_{nullptr};
};

}  // namespace exe::fibers
