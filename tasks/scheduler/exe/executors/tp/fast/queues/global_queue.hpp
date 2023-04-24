#pragma once

#include <exe/executors/task.hpp>

#include <wheels/intrusive/forward_list.hpp>

#include <twist/ed/stdlike/mutex.hpp>

#include <span>

namespace exe::executors::tp::fast {

// Unbounded queue shared between workers

class GlobalQueue {
 public:
  void Push(IntrusiveTask* item) {
    std::lock_guard guard(mutex_);
    list_.PushBack(item);
  }

  void Offload(std::span<IntrusiveTask*> buffer) {
    wheels::IntrusiveForwardList<IntrusiveTask> tmp_list;
    for (auto item : buffer) {
      tmp_list.PushBack(item);
    }

    std::lock_guard guard(mutex_);
    list_.Append(tmp_list);
  }

  // Returns nullptr if queue is empty
  IntrusiveTask* TryPop() {
    std::lock_guard guard(mutex_);
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
    return sz;
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  wheels::IntrusiveForwardList<IntrusiveTask> list_;
};

}  // namespace exe::executors::tp::fast
