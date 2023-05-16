#pragma once

#include <exe/futures/containers/container.hpp>

namespace exe::futures::detail {

template <SomeFuture Producer>
struct BoxedContainer final : detail::Container<typename Producer::ValueType>,
                              IConsumer<typename Producer::ValueType> {
  using ValueType = typename Producer::ValueType;

 public:
  explicit BoxedContainer(Producer p)
      : producer_(std::move(p)) {
  }

  void Start(IConsumer<ValueType>* consumer) override {
    consumer_ = consumer;
    producer_.Start(this);
  }

 private:
  void Consume(Output<ValueType> output) noexcept override {
    consumer_->Complete(std::move(output));
    delete this;
  }

  Producer producer_;
  IConsumer<ValueType>* consumer_;
};

}  // namespace exe::futures::detail