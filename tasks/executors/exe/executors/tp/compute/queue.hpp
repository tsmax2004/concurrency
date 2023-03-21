#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <optional>
#include <deque>

namespace exe::executors::tp::compute {

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue

template <typename T>
class UnboundedBlockingQueue {
  using IntrusiveList = wheels::IntrusiveList<IntrusiveTask>;

 public:
  bool Put(T value) {
    std::lock_guard guard(mutex_);
    if (is_closed_) {
      return false;
    }

    buffer_.PushBack(std::move(value));
    is_empty_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock guard(mutex_);

    while (buffer_.IsEmpty()) {
      if (is_closed_) {
        return std::nullopt;
      }
      is_empty_.wait(guard);
    }

    auto value = std::move(buffer_.PopFront());
    return std::move(value);
  }

  void Close() {
    std::lock_guard guard(mutex_);
    is_closed_ = true;
    is_empty_.notify_all();
  }

  ~UnboundedBlockingQueue() {
    buffer_.UnlinkAll();
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable is_empty_;

  IntrusiveList buffer_;
  bool is_closed_{false};
};

}  // namespace exe::executors::tp::compute
