#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(size_t capacity)
      : free_space_sem_(capacity),
        objects_sem_(0),
        locked_semaphore_(1) {
  }

  void Put(T value) {
    auto free_space_token = free_space_sem_.Acquire();
    auto locked_token = locked_semaphore_.Acquire();

    buffer_.emplace_back(std::move(value));

    objects_sem_.Release(std::move(free_space_token));
    locked_semaphore_.Release(std::move(locked_token));
  }

  T Take() {
    auto object_token = objects_sem_.Acquire();
    auto locked_token = locked_semaphore_.Acquire();

    auto obj = std::move(buffer_.front());
    buffer_.pop_front();

    free_space_sem_.Release(std::move(object_token));
    locked_semaphore_.Release(std::move(locked_token));

    return std::move(obj);
  }

 private:
  // Tags
  struct BufferTag {};
  struct AccessTag {};

 private:
  // Buffer
  std::deque<T> buffer_;

  TaggedSemaphore<BufferTag> free_space_sem_;
  TaggedSemaphore<BufferTag> objects_sem_;
  TaggedSemaphore<AccessTag> locked_semaphore_;
};
