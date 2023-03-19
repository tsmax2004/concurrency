#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/spin.hpp>

namespace exe::threads {

// Test-and-TAS spinlock

// class SpinLock {
//  public:
//   void Lock() {
//     twist::ed::SpinWait spin_wait;
//     while (!TryLock()) {
//       while (locked_.load()) {
//         spin_wait();
//       }
//     }
//   }
//
//   bool TryLock() {
//     return !locked_.exchange(true);
//   }
//
//   void Unlock() {
//     locked_.store(false);
//   }
//
//   // BasicLockable
//
//   void lock() {  // NOLINT
//     Lock();
//   }
//
//   bool try_lock() {  // NOLINT
//     return TryLock();
//   }
//
//   void unlock() {  // NOLINT
//     Unlock();
//   }
//
//  private:
//   twist::ed::stdlike::atomic<bool> locked_{false};
// };

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
    auto prev_tail = tail_.exchange(waiter);
    if (prev_tail == nullptr) {
      return;
    }
    prev_tail->next_.store(waiter);

    twist::ed::SpinWait spin_wait;
    while (!waiter->is_owner_.load()) {
      spin_wait();
    }
  }

  void Release(Guard* owner) {
    auto tmp_owner = owner;
    if (tail_.compare_exchange_strong(tmp_owner, nullptr)) {
      return;
    }

    twist::ed::SpinWait spin_wait;
    while (owner->next_.load() == nullptr) {
      spin_wait();
    }
    owner->next_.load()->is_owner_.store(true);
  }

 private:
  twist::ed::stdlike::atomic<Guard*> tail_{nullptr};
};

}  // namespace exe::threads
