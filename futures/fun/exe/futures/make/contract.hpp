#pragma once

#include <exe/futures/types/future.hpp>
#include <exe/futures/state/shared_state.hpp>

#include <exe/result/types/result.hpp>

#include <exe/executors/executor.hpp>
#include <exe/executors/inline.hpp>

#include <tuple>

namespace exe::futures {

template <typename T>
class Promise {
 public:
  explicit Promise(detail::SharedBuffer<T>* buf)
      : buffer_ptr_(buf) {
  }

  void Set(Result<T> result) && {
    buffer_ptr_->Set(std::move(result));
  }

  void SetValue(T value) && {
    buffer_ptr_->SetValue(std::forward<T>(value));
  }

  void SetError(Error err) && {
    buffer_ptr_->SetException(std::move(err));
  }

 private:
  detail::SharedBuffer<T>* buffer_ptr_;
};

template <typename T>
std::tuple<Future<T>, Promise<T>> Contract(executors::IExecutor& exe) {
  auto buffer_ptr = new detail::SharedBuffer<T>(exe);
  return {Future(buffer_ptr), Promise(buffer_ptr)};
}

template <typename T>
std::tuple<Future<T>, Promise<T>> Contract() {
  return Contract<T>(executors::Inline());
}

}  // namespace exe::futures
