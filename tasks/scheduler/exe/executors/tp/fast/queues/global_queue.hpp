#pragma once

#include <exe/executors/task.hpp>

#include <wheels/intrusive/forward_list.hpp>

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/atomic.hpp>

#include <span>

namespace exe::executors::tp::fast {

// Unbounded queue shared between workers

class GlobalQueue {
 public:
  void Push(IntrusiveTask* item) {
    std::lock_guard guard(mutex_);
    list_.PushBack(item);
    size_.fetch_add(1);
  }

  void Offload(std::span<IntrusiveTask*> buffer) {
    wheels::IntrusiveForwardList<IntrusiveTask> tmp_list;
    for (auto item : buffer) {
      tmp_list.PushBack(item);
    }

    std::lock_guard guard(mutex_);
    list_.Append(tmp_list);
    size_.fetch_add(buffer.size());
  }

  // Returns nullptr if queue is empty
  IntrusiveTask* TryPop() {
    std::lock_guard guard(mutex_);
    if (size_.load() == 0) {
      return nullptr;
    }

    size_.fetch_sub(1);
    return list_.PopFront();
  }

  // Returns number of items in `out_buffer`
  size_t Grab(std::span<IntrusiveTask*> out_buffer, size_t workers) {
    std::lock_guard guard(mutex_);
    if (list_.IsEmpty()) {
      return 0;
    }

    size_t sz = std::min(list_.Size(), list_.Size() / workers + 1);
    sz = std::min(sz, out_buffer.size());
    for (size_t i = 0; i < sz; ++i) {
      out_buffer[i] = list_.PopFront();
    }

    size_.fetch_sub(sz);
    return sz;
  }

  bool HasItems() {
    return size_.load() > 0;
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  wheels::IntrusiveForwardList<IntrusiveTask> list_;
  twist::ed::stdlike::atomic<size_t> size_;
};

}  // namespace exe::executors::tp::fast
