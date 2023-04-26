#pragma once

#include <exe/executors/task.hpp>

#include <array>
#include <span>

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::executors::tp::fast {

// Single producer / multiple consumers bounded queue
// for local tasks

template <size_t Capacity>
class WorkStealingQueue {
  struct Slot {
    twist::ed::stdlike::atomic<IntrusiveTask*> item{nullptr};
  };

 public:
  // Single producer

  bool TryPush(IntrusiveTask* item) {
    IntrusiveTask* expected = nullptr;
    if (!buffer_[tail_.load(std::memory_order_relaxed) % Capacity]
             .item.compare_exchange_strong(expected, item,
                                           std::memory_order_relaxed,
                                           std::memory_order_relaxed)) {
      return false;
    }

    tail_.fetch_add(1, std::memory_order_release);
    return true;
  }

  // For grabbing from global queue / for stealing
  // Should always succeed
  void PushMany(std::span<TaskBase*> buffer) {
    auto start = tail_.load(std::memory_order_relaxed);
    auto finish = start + buffer.size();
    for (size_t current_tail = start; current_tail < finish; ++current_tail) {
      buffer_[current_tail % Capacity].item.store(buffer[current_tail - start],
                                                  std::memory_order_relaxed);
      tail_.fetch_add(1, std::memory_order_release);
    }

    //    auto crt_tail = tail_.load(std::memory_order_relaxed);
    //    for (size_t i = crt_tail; i < crt_tail + buffer.size(); ++i) {
    //      buffer_[i % Capacity].item.store(buffer[i],
    //      std::memory_order_relaxed);
    //    }
    //    tail_.fetch_add(buffer.size(), std::memory_order_release);
  }

  // Multiple consumers

  // Returns nullptr if queue is empty
  TaskBase* TryPop() {
    IntrusiveTask* item[1];
    std::span<IntrusiveTask*, 1> buf(item);
    if (Grab(buf) == 0) {
      return nullptr;
    }
    return buf[0];
  }

  // For stealing and for offloading to global queue
  // Returns number of tasks in `out_buffer`
  size_t Grab(std::span<TaskBase*> out_buffer) {
    auto grab_head = head_.load(std::memory_order_relaxed);
    auto grab_size = std::min(out_buffer.size(), Size());
    while (!head_.compare_exchange_strong(grab_head, grab_head + grab_size,
                                          std::memory_order_relaxed,
                                          std::memory_order_relaxed)) {
      grab_size = std::min(out_buffer.size(), Size());
    }
    if (buffer_[grab_head % Capacity].item.load(std::memory_order_relaxed) ==
        nullptr) {
      return 0;
    }

    MoveItems(grab_head, grab_head + grab_size, out_buffer);
    return grab_size;
  }

  size_t Size() {
    return tail_.load(std::memory_order_acquire) -
           head_.load(std::memory_order_relaxed);
  }

  bool HasItems() {
    return Size() > 0;
  }

 private:
  void MoveItems(size_t from, size_t to, std::span<IntrusiveTask*> out) {
    for (auto i = from; i < to; ++i) {
      out[i - from] = buffer_[i % Capacity].item.exchange(
          nullptr, std::memory_order_relaxed);
    }
  }

  std::array<Slot, Capacity> buffer_;

  twist::ed::stdlike::atomic<size_t> head_{0};
  twist::ed::stdlike::atomic<size_t> tail_{0};
};

}  // namespace exe::executors::tp::fast
