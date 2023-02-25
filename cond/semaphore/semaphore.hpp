#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdlib>

class Semaphore {
 public:
  explicit Semaphore(size_t tokens)
      : tokens_(tokens) {
  }

  void Acquire() {
    std::unique_lock guard(mutex_);
    while (tokens_ == 0) {
      is_empty_.wait(guard);
    }
    --tokens_;
  }

  void Release() {
    std::lock_guard guard(mutex_);
    ++tokens_;
    is_empty_.notify_one();
  }

 private:
  size_t tokens_;
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable is_empty_;
};
