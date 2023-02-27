#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdlib>

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t participants)
      : participants_(participants) {
  }

  void ArriveAndWait() {
    std::unique_lock guard(mutex_);

    if (is_even_) {
      EvenParking(guard);
    } else {
      OddParking(guard);
    }
  }

 private:
  void EvenParking(std::unique_lock<twist::ed::stdlike::mutex>& guard) {
    ++arrived_even_;
    if (arrived_even_ == participants_) {
      AllArrived();
    }
    while (arrived_even_ < participants_) {
      all_arrived_.wait(guard);
    }
  }

  void OddParking(std::unique_lock<twist::ed::stdlike::mutex>& guard) {
    ++arrived_odd_;
    if (arrived_odd_ == participants_) {
      AllArrived();
    }
    while (arrived_odd_ < participants_) {
      all_arrived_.wait(guard);
    }
  }

  void AllArrived() {
    is_even_ = not is_even_;
    if (is_even_) {
      arrived_even_ = 0;
    } else {
      arrived_odd_ = 0;
    }
    all_arrived_.notify_all();
  }

  size_t participants_;
  size_t arrived_even_{0};
  size_t arrived_odd_{0};
  bool is_even_{true};

  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable all_arrived_;
};
