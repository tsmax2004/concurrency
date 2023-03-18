#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/spin.hpp>

/*
 * Scalable Queue SpinLock
 *
 * Usage:
 *
 * QueueSpinLock spinlock;
 *
 * {
 *   QueueSpinLock::Guard guard(spinlock);  // <-- Acquire
 *   // <-- Critical section
 * }  // <-- Release
 *
 */

class QueueSpinLock {
 public:
  class Guard {
    friend class QueueSpinLock;

   public:
    explicit Guard(QueueSpinLock& host)
        : host_(host) {
      host_.Acquire(this);
    }

    ~Guard() {
      host_.Release(this);
    }

   private:
    QueueSpinLock& host_;
    twist::ed::stdlike::atomic<Guard*> next_;
    twist::ed::stdlike::atomic<bool> is_owner_;
  };

 private:
  void Acquire(Guard* waiter) {
    Guard* null1 = nullptr;
    Guard* null2 = nullptr;

    while (!tail_.compare_exchange_strong(null1, waiter) ||
           !tail_.load()->next_.compare_exchange_strong(null2, waiter)) {
      null1 = nullptr;
      null2 = nullptr;
    }
    tail_.exchange(waiter);

    twist::ed::SpinWait spin_wait;
    while (waiter->is_owner_.load()) {
      spin_wait();
    }
  }

  void Release(Guard* owner) {
    if (tail_.compare_exchange_strong(owner, nullptr) ||
        owner->next_.load() == nullptr) {
      return;
    }

    owner->next_.load()->is_owner_.store(true);
  }

 private:
  twist::ed::stdlike::atomic<Guard*> tail_;
};
