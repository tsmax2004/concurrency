#pragma once

#include <optional>

#include <exe/futures/model/thunk.hpp>

namespace exe::futures::thunks {

template <typename T>
concept ApplyTraits = requires(T traits,
                               Result<typename T::InputValueType> input,
                               typename T::MapFun fun) {
  typename T::Producer;
  typename T::MapFun;
  typename T::InputValueType;
  typename T::ValueType;

  T::Shortcut(input);
  T::ComputeResult(input, fun);
};

template <ApplyTraits ApplyTraits>
struct [[nodiscard]] Apply final
    : IConsumer<typename ApplyTraits::InputValueType>,
      executors::IntrusiveTask {
  using Producer = typename ApplyTraits::Producer;
  using MapFun = typename ApplyTraits::MapFun;
  using InputValueType = typename ApplyTraits::InputValueType;
  using ValueType = typename ApplyTraits::ValueType;

 public:
  explicit Apply(Producer p, MapFun fun)
      : producer_(std::move(p)),
        fun_(std::move(fun)) {
  }

  // Non-copyable
  Apply(const Apply&) = delete;

  // Movable
  Apply(Apply&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    consumer_ = consumer;
    producer_.Start(this);
  }

 protected:
  // IConsumer
  void Consume(Output<InputValueType> input) noexcept override {
    if (ApplyTraits::Shortcut(input.result)) {
      auto result =
          ApplyTraits::ComputeResult(std::move(input.result), std::move(fun_));
      consumer_->Complete({std::move(result), std::move(input.context)});
      return;
    }

    input_.emplace(std::move(input));
    input.context.exe->Submit(this);
  }

  // IntrusiveTask
  void Run() noexcept override {
    auto result =
        ApplyTraits::ComputeResult(std::move(input_->result), std::move(fun_));
    consumer_->Complete({std::move(result), std::move(input_->context)});
  }

  Producer producer_;
  MapFun fun_;
  IConsumer<ValueType>* consumer_;
  std::optional<Output<InputValueType>> input_;
};

}  // namespace exe::futures::thunks
