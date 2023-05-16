#pragma once

#include <exe/futures/types/future.hpp>
#include <exe/futures/types/eager.hpp>

#include <exe/result/types/result.hpp>
#include <exe/result/make/ok.hpp>
#include <exe/result/make/err.hpp>

#include <exe/futures/containers/shared_state.hpp>

#include <tuple>

namespace exe::futures {

template <typename T>
class Promise {
 public:
  explicit Promise(detail::SharedState<T>* state)
      : shared_state_(state) {
  }

  void Set(Result<T> result) && {
    shared_state_->Complete(std::move(result));
  }

  void SetValue(T value) && {
    shared_state_->Complete(result::Ok(std::move(value)));
  }

  void SetError(Error err) && {
    shared_state_->Complete(result::Err(std::move(err)));
  }

 private:
  detail::SharedState<T>* shared_state_;
};

template <typename T>
std::tuple<EagerFuture<T>, Promise<T>> Contract() {
  auto* container = new detail::SharedState<T>();
  return {EagerFuture<T>(container), Promise<T>(container)};
}

}  // namespace exe::futures
