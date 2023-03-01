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
    auto old_epoch = epoch_;

    if (++arrived_ == participants_) {
      ++epoch_;
      arrived_ = 0;
      all_arrived_.notify_all();
    }

    while (old_epoch == epoch_) {
      all_arrived_.wait(guard);
    }
  }

 private:
  size_t participants_;
  size_t arrived_{0};
  size_t epoch_{0};

  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable all_arrived_;
};
