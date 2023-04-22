#pragma once

#include <cstdlib>
#include <utility>

#include <exe/futures/state/shared_state.hpp>
#include <exe/futures/callback/callback.hpp>

#include <exe/executors/executor.hpp>

namespace exe::futures {

template <typename T>
struct [[nodiscard]] Future {
  using ValueType = T;

  explicit Future(detail::SharedBuffer<T>* buf)
      : buffer_ptr_(buf) {
  }

  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Non-copy-assignable
  Future& operator=(Future&) = delete;

  // Movable
  Future(Future&& other)
      : buffer_ptr_(std::move(other.buffer_ptr_)){};

  void Consume(detail::Callback<T> callback) {
    buffer_ptr_->Consume(std::move(callback));
  }

  void Via(executors::IExecutor& exe) {
    buffer_ptr_->Via(exe);
  }

  executors::IExecutor& GetExecutor() {
    return buffer_ptr_->GetExecutor();
  }

 private:
  detail::SharedBuffer<T>* buffer_ptr_;
};

}  // namespace exe::futures
