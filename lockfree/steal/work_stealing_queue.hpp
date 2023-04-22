#pragma once

#include <array>
#include <span>

#include <twist/ed/stdlike/atomic.hpp>

// Single-Producer / Multi-Consumer Bounded Ring Buffer

template <typename T, size_t Capacity>
class WorkStealingQueue {
  struct Slot {
    T* item{nullptr};
    twist::ed::stdlike::atomic<bool> is_filled{false};
  };

 public:
  bool TryPush(T* item) {
    if (buffer_[tail_ % Capacity].is_filled.load(std::memory_order_relaxed)) {
      return false;
    }

    buffer_[tail_ % Capacity].item = item;
    buffer_[tail_ % Capacity].is_filled.store(true, std::memory_order_release);

    tail_.fetch_add(1, std::memory_order_relaxed);
    return true;
  }

  // Returns nullptr if queue is empty
  T* TryPop() {
    T* item[1];
    std::span<T*, 1> buf(item);
    if (Grab(buf) == 0) {
      return nullptr;
    }
    return buf[0];
  }

  // Returns number of tasks
  size_t Grab(std::span<T*> out_buffer) {
    auto grab_head = head_.load(std::memory_order_relaxed);
    auto grab_size = std::min(out_buffer.size(), Size());
    while (!head_.compare_exchange_strong(grab_head, grab_head + grab_size,
                                          std::memory_order_acquire,
                                          std::memory_order_relaxed)) {
      grab_size = std::min(out_buffer.size(), Size());
    }
    if (!buffer_[grab_head % Capacity].is_filled.load(
            std::memory_order_relaxed)) {
      return 0;
    }

    MoveItems(grab_head, grab_head + grab_size, out_buffer);
    return grab_size;
  }

 private:
  size_t Size() {
    return tail_.load(std::memory_order_relaxed) -
           head_.load(std::memory_order_relaxed);
  }

  void MoveItems(size_t from, size_t to, std::span<T*> out) {
    for (auto i = from; i < to; ++i) {
      out[i - from] = buffer_[i % Capacity].item;
      buffer_[i % Capacity].item = nullptr;
      buffer_[i % Capacity].is_filled.store(false, std::memory_order_release);
    }
  }

  std::array<Slot, Capacity> buffer_;

  twist::ed::stdlike::atomic<size_t> head_{0};
  twist::ed::stdlike::atomic<size_t> tail_{0};
};
