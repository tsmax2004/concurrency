#pragma once

#include <cstdlib>

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

class WaitGroup {
 public:
  // += count
  void Add(size_t count) {
    std::lock_guard guard(mutex_);
    counter_ += count;
  }

  // =- 1
  void Done() {
    std::lock_guard guard(mutex_);
    if (--counter_ == 0) {
      is_all_done_.notify_all();
    }
  }

  // == 0
  // One-shot
  void Wait() {
    std::unique_lock guard(mutex_);
    while (counter_ > 0) {
      is_all_done_.wait(guard);
    }
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable is_all_done_;
  size_t counter_{0};
};
