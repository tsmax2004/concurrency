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
    EventAwaiter awaiter(*this);
    Suspend(awaiter);
  }

  void Fire() {
    void* old_state = event_state_.exchange(this);
    if (old_state == nullptr || old_state == this) {
      return;
    }

    auto* awaiter = static_cast<EventAwaiter*>(old_state);
    while (awaiter != nullptr) {
      auto* next = awaiter->next_;
      awaiter->AwaitResume();
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

    bool AwaitReady() override {
      return event_.IsFired();
    }

    bool AwaitSuspend(FiberHandle fiber) override {
      fiber_ = fiber;

      void* old = event_.event_state_.load();

      do {
        if (AwaitReady()) {  // fired
          return false;
        }

        next_ = static_cast<EventAwaiter*>(old);
      } while (!event_.event_state_.compare_exchange_strong(old, this));

      return true;
    }

    void AwaitResume() override {
      fiber_.Schedule();
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
