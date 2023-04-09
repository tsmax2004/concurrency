#pragma once

#include <stdlike/shared_buffer.hpp>

#include <cassert>

namespace stdlike {

template <typename T>
class Future {
  template <typename U>
  friend class Promise;

 public:
  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Movable
  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  // One-shot (although it can be multiple-shots)
  // Wait for result (value or exception)
  T Get() {
    auto result = buffer_ptr_->Get();
    if (!result.has_value()) {
      std::rethrow_exception(result.error());
    }

    return std::forward<T>(result.value());
  }

 private:
  explicit Future(detail::SharedBuffer<T>* buffer_ptr)
      : buffer_ptr_(buffer_ptr) {
  }

 private:
  detail::SharedBuffer<T>* buffer_ptr_;
};

}  // namespace stdlike
