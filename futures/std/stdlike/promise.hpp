#pragma once

#include <stdlike/future.hpp>
#include <stdlike/buffer.hpp>

#include <memory>

namespace stdlike {

template <typename T>
class Promise {
  using BufferPtr = std::shared_ptr<detail::Buffer<T>>;

 public:
  Promise() {
    buffer_ptr_ = std::make_shared<detail::Buffer<T>>();
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
  void SetValue(T value) {
    CheckSetException();
    std::lock_guard guard(buffer_ptr_->mutex);

    buffer_ptr_->value = std::move(value);
    buffer_ptr_->is_ready = true;
    buffer_ptr_->is_ready_cv.notify_one();
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr exception) {
    CheckSetException();
    std::lock_guard guard(buffer_ptr_->mutex);

    buffer_ptr_->value = std::move(exception);
    buffer_ptr_->is_ready = true;
    buffer_ptr_->is_exception = true;
    buffer_ptr_->is_ready_cv.notify_one();
  }

 private:
  void CheckSetException() {
    if (is_set_) {
      throw std::logic_error(
          "The shared state already stores a value or exception.");
    }
    is_set_ = true;
  }

  BufferPtr buffer_ptr_;
  bool is_future_made_{false};
  bool is_set_{false};
};

}  // namespace stdlike
