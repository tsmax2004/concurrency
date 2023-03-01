#pragma once

#include <stdlike/buffer.hpp>

#include <memory>
#include <cassert>

namespace stdlike {

template <typename T>
class Future {
  using BufferPtr = std::shared_ptr<detail::Buffer<T>>;

  template <typename U>
  friend class Promise;

 public:
  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Movable
  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  // TODO: One-shot
  // Wait for result (value or exception)
  T Get() {
    std::unique_lock guard(buffer_ptr_->mutex);
    while (!buffer_ptr_->is_ready) {
      buffer_ptr_->is_ready_cv.wait(guard);
    }

    if (buffer_ptr_->is_exception) {
      std::rethrow_exception(std::get<std::exception_ptr>(buffer_ptr_->value));
    }
    return std::move(std::get<T>(buffer_ptr_->value));
  }

 private:
  explicit Future(BufferPtr buffer_ptr)
      : buffer_ptr_(buffer_ptr) {
  }

 private:
  BufferPtr buffer_ptr_;
};

}  // namespace stdlike
