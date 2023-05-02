#pragma once

#include <wheels/intrusive/forward_list.hpp>
#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <exe/executors/task.hpp>

namespace exe::executors::tp::compute::support {

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue
// for intrusive tasks

template <typename T>
class IntrusiveUnboundedBlockingQueue {
  using IntrusiveList = wheels::IntrusiveList<T>;

 public:
  bool Put(T* node) {
    std::lock_guard guard(mutex_);
    if (is_closed_) {
      return false;
    }

    buffer_.PushBack(node);
    is_empty_.notify_one();
    return true;
  }

  T* Take() {
    std::unique_lock guard(mutex_);

    while (buffer_.IsEmpty()) {
      if (is_closed_) {
        return nullptr;
      }
      is_empty_.wait(guard);
    }

    auto value = buffer_.PopFront();
    return value;
  }

  void Close() {
    std::lock_guard guard(mutex_);

    is_closed_ = true;
    is_empty_.notify_all();
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable is_empty_;

  IntrusiveList buffer_;
  bool is_closed_{false};
};

}  // namespace exe::executors::tp::compute::support
