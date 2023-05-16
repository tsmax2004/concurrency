#pragma once

#include <optional>

#include <exe/futures/model/thunk.hpp>

#include <exe/futures/state/box.hpp>

namespace exe::futures::thunks {

template <typename T>
struct [[nodiscard]] Boxed : IConsumer<T> {
  using ValueType = T;

 public:
  // Auto-boxing
  template <Thunk Thunk>
  explicit Boxed(Thunk thunk) {
    shared_state_ = new detail::BoxedSharedState(std::move(thunk));
  }

  // Non-copyable
  Boxed(const Boxed&) = delete;

  // Movable
  Boxed(Boxed&&) = default;

  void Start(IConsumer<T>* consumer) {
    consumer_ = consumer;
    shared_state_->Start(this);
  }

 private:
  void Consume(Output<ValueType> output) noexcept override {
    consumer_->Complete(std::move(output));
  }

  detail::SharedState<ValueType>* shared_state_;
  IConsumer<ValueType>* consumer_;
};

}  // namespace exe::futures::thunks
