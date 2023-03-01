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

  // TODO: One-shot
  Future<T> MakeFuture() {
    return Future<T>(buffer_ptr_);
  }

  // TODO: One-shot
  // Fulfill promise with value
  void SetValue(T value) {
    std::lock_guard guard(buffer_ptr_->mutex);

    buffer_ptr_->value = std::move(value);
    buffer_ptr_->is_ready = true;
    buffer_ptr_->is_ready_cv.notify_one();
  }

  // TODO: One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr exception) {
    std::lock_guard guard(buffer_ptr_->mutex);

    buffer_ptr_->value = std::move(exception);
    buffer_ptr_->is_ready = true;
    buffer_ptr_->is_ready_cv.notify_one();
  }

 private:
  BufferPtr buffer_ptr_;
};

}  // namespace stdlike
