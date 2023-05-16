#pragma once

#include <exe/futures/thunks/combine/seq/base.hpp>

namespace exe::futures::thunks {

template <Thunk Producer, typename FutureTypeT = typename Producer::ValueType,
          typename ValueTypeT = typename FutureTypeT::ValueType>
struct [[nodiscard]] Flatten : IConsumer<ValueTypeT>,
                               IConsumer<FutureTypeT> {
  using FutureType = FutureTypeT;
  using ValueType = ValueTypeT;

 public:
  explicit Flatten(Producer p)
      : producer_(std::move(p)) {
  }

  // Non-copyable
  Flatten(const Flatten&) = delete;

  // Movable
  Flatten(Flatten&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    consumer_ = consumer;
    producer_.Start(this);
  }

 private:
  void Consume(Output<FutureType> future_output) noexcept override {
    future_output_.emplace(std::move(future_output));
    future_output_.value().result.value().Start(this);
  }

  void Consume(Output<ValueType> output) noexcept override {
    output.context = std::move(future_output_->context);
    consumer_->Complete(std::move(output));
  }

  Producer producer_;
  IConsumer<ValueType>* consumer_;
  std::optional<Output<FutureType>> future_output_;
};

}  // namespace exe::futures::thunks
