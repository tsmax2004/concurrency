#pragma once

#include <optional>

#include <exe/futures/model/thunk.hpp>

namespace exe::futures::thunks {

template <Thunk Producer, typename ValueType>
struct [[nodiscard]] SeqThunk : IConsumer<typename Producer::ValueType>,
                                executors::IntrusiveTask {
  using InputValueType = typename Producer::ValueType;

 public:
  explicit SeqThunk(Producer p)
      : producer_(std::move(p)) {
  }

  // Non-copyable
  SeqThunk(const SeqThunk&) = delete;

  // Movable
  SeqThunk(SeqThunk&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    consumer_ = std::move(consumer);
    producer_.Start(this);
  }

 protected:
  // IConsumer
  void Consume(Output<InputValueType> input) noexcept override {
    input_.emplace(std::move(input));
    input.context.exe->Submit(this);
  }

  // IntrusiveTask
  void Run() noexcept override {
    auto result = std::move(PerformResult());
    consumer_->Complete({std::move(result), std::move(input_->context)});
  }

  virtual Result<ValueType> PerformResult() = 0;

  Producer producer_;
  IConsumer<ValueType>* consumer_;
  std::optional<Output<InputValueType>> input_;
};

}  // namespace exe::futures::thunks
