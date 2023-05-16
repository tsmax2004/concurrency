#pragma once

#include <exe/futures/model/thunk.hpp>
#include <exe/result/make/ok.hpp>

namespace exe::futures::thunks {

template <typename T>
struct [[nodiscard]] Value {
  using ValueType = T;

 public:
  explicit Value(ValueType v)
      : value_(std::move(v)) {
  }

  // Non-copyable
  Value(const Value&) = delete;

  // Movable
  Value(Value&& other)
      : value_(std::move(other.value_)){};

  void Start(IConsumer<ValueType>* consumer) {
    consumer->Complete(result::Ok(std::move(value_)));
  }

 private:
  ValueType value_;
};

}  // namespace exe::futures::thunks
