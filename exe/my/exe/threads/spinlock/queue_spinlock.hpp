#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/threads/support/spin_wait.hpp>

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

namespace exe::threads {

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

    twist::ed::stdlike::atomic<Guard*> next_{nullptr};
    twist::ed::stdlike::atomic<bool> is_owner_{false};
  };

 private:
  void Acquire(Guard* waiter) {
    auto prev_tail = tail_.exchange(waiter, std::memory_order_acquire);
    if (prev_tail == nullptr) {
      return;
    }
    prev_tail->next_.store(waiter, std::memory_order_relaxed);

    support::SpinWait spin_wait;
    while (!waiter->is_owner_.load(std::memory_order_acquire)) {
      spin_wait();
    }
  }

  void Release(Guard* owner) {
    auto tmp_owner = owner;
    if (tail_.compare_exchange_strong(tmp_owner, nullptr,
                                      std::memory_order_release,
                                      std::memory_order_relaxed)) {
      return;
    }

    support::SpinWait spin_wait;
    while (owner->next_.load(std::memory_order_relaxed) == nullptr) {
      spin_wait();
    }
    owner->next_.load(std::memory_order_relaxed)
        ->is_owner_.store(true, std::memory_order_release);
  }

 private:
  twist::ed::stdlike::atomic<Guard*> tail_{nullptr};
};

}  // namespace exe::threads