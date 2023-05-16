#pragma once

#include <exe/futures/model/thunk.hpp>

#include <cstdlib>

namespace exe::futures::thunks {

template <Thunk Wrapped>
struct [[nodiscard]] Via : IConsumer<typename Wrapped::ValueType> {
  using ValueType = typename Wrapped::ValueType;

 public:
  Via(Wrapped thunk, executors::IExecutor* exe)
      : wrapped_thunk_(std::move(thunk)),
        exe_(exe) {
  }

  // Non-copyable
  Via(const Via&) = delete;

  // Movable
  Via(Via&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    consumer_ = std::move(consumer);
    wrapped_thunk_.Start(this);
  }

 private:
  void Consume(Output<ValueType> output) noexcept override {
    output.context.exe = exe_;
    consumer_->Complete(std::move(output));
  }

  Wrapped wrapped_thunk_;
  executors::IExecutor* exe_;
  IConsumer<ValueType>* consumer_;
};

}  // namespace exe::futures::thunks
