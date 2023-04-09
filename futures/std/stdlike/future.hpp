#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <stdlike/shared_buffer.hpp>
#include <stdlike/callback.hpp>

#include <cassert>

namespace stdlike {

template <typename T>
struct GetValueCallback : detail::Callback<T> {
 public:
  void operator()(detail::Result<T> result) override {
    result_ = std::move(result);
    is_ready_.store(true);
    is_ready_.notify_one();
  }

  T GetValue() {
    is_ready_.wait(false);

    if (!result_.has_value()) {
      std::rethrow_exception(result_.error());
    }
    return std::forward<T>(result_.value());
  }

 private:
  detail::Result<T> result_{tl::make_unexpected(nullptr)};
  twist::ed::stdlike::atomic<bool> is_ready_{false};
};

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
    get_called_ = true;

    GetValueCallback<T> callback;
    buffer_ptr_->Consume(&callback);
    return std::forward<T>(callback.GetValue());
  }

  ~Future() {
    if (!get_called_) {
      buffer_ptr_->Consume(nullptr);
    }
  }

 private:
  explicit Future(detail::SharedBuffer<T>* buffer_ptr)
      : buffer_ptr_(buffer_ptr) {
  }

  bool get_called_{false};
  detail::SharedBuffer<T>* buffer_ptr_;
};

}  // namespace stdlike
