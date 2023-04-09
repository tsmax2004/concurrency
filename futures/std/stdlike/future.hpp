#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <stdlike/shared_buffer.hpp>
#include <stdlike/callback.hpp>

#include <cassert>

namespace stdlike {

template <typename T>
struct GetValueCallback : detail::Callback<T> {
 public:
  void operator()(detail::Result<T> result) override {
    result_ = std::move(result);

    auto key = twist::ed::PrepareWake(is_ready_);
    is_ready_.store(1);
    twist::ed::WakeOne(key);
  }

  T GetValue() {
    twist::ed::Wait(is_ready_, 0);

    if (!result_.has_value()) {
      std::rethrow_exception(result_.error());
    }
    return std::forward<T>(result_.value());
  }

 private:
  detail::Result<T> result_{tl::make_unexpected(nullptr)};
  twist::ed::stdlike::atomic<uint32_t> is_ready_{0};
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
  Future(Future&& other)
      : get_called_(other.get_called_),
        buffer_ptr_(other.buffer_ptr_) {
    other.buffer_ptr_ = nullptr;
  }
  Future& operator=(Future&& other) {
    buffer_ptr_ = other.buffer_ptr_;
    get_called_ = other.get_called_;
    other.buffer_ptr_ = nullptr;
  };

  // One-shot (although it can be multiple-shots)
  // Wait for result (value or exception)
  T Get() {
    get_called_ = true;

    GetValueCallback<T> callback;
    buffer_ptr_->Consume(&callback);
    return std::forward<T>(callback.GetValue());
  }

  ~Future() {
    if (buffer_ptr_ != nullptr && !get_called_) {
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
