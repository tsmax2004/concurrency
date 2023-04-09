#pragma once

#include <stdlike/future.hpp>
#include <stdlike/shared_buffer.hpp>
#include <stdlike/result.hpp>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise() {
    buffer_ptr_ = new detail::SharedBuffer<T>();
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() {
    if (is_future_made_) {
      throw std::logic_error(
          "MakeFuture() has already been called on a promise with the same "
          "shared state.");
    }
    is_future_made_ = true;
    return Future<T>(buffer_ptr_);
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T&& value) {
    CheckSetException();
    buffer_ptr_->SetValue(std::forward<T>(value));
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr exception) {
    CheckSetException();
    buffer_ptr_->SetException(std::move(exception));
  }

 private:
  void CheckSetException() {
    if (is_set_) {
      throw std::logic_error(
          "The shared state already stores a value or exception.");
    }
    is_set_ = true;
  }

  detail::SharedBuffer<T>* buffer_ptr_;
  bool is_future_made_{false};
  bool is_set_{false};
};

}  // namespace stdlike
