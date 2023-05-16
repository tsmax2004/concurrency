#pragma once

#include <exe/futures/model/thunk.hpp>

#include <exe/futures/state/eager.hpp>

namespace exe::futures::thunks {

template <typename T>
struct [[nodiscard]] Eager : IConsumer<T> {
  using ValueType = T;

 public:
  explicit Eager(detail::SharedState<ValueType>* p)
      : producer_(p) {
  }

  // Non-copyable
  Eager(const Eager&) = default;

  // Movable
  Eager(Eager&& other) = default;

  void Start(IConsumer<ValueType>* consumer) {
    consumer_ = consumer;
    producer_->Start(this);
  }

 private:
  // IConsumer
  void Consume(Output<ValueType> output) noexcept override {
    consumer_->Complete(std::move(output));
  }

  IConsumer<ValueType>* consumer_;
  detail::SharedState<ValueType>* producer_;
};

}  // namespace exe::futures::thunks
