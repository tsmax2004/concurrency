#pragma once

#include <exe/futures/model/thunk.hpp>
#include <exe/result/make/err.hpp>

namespace exe::futures::thunks {

template <typename T>
struct [[nodiscard]] Failure {
  using ValueType = T;

 public:
  explicit Failure(Error err)
      : err_(std::move(err)) {
  }

  // Non-copyable
  Failure(const Failure&) = delete;

  // Movable
  Failure(Failure&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    consumer->Complete(result::Err(std::move(err_)));
  }

 private:
  Error err_;
};

}  // namespace exe::futures::thunks
