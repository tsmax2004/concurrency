#pragma once

#include <exe/futures/model/thunk.hpp>

namespace exe::futures::thunks {

namespace detail {

template <Thunk Producer1, Thunk Producer2,
          typename ValueTypeT = typename Producer1::ValueType>
struct FirstContainer : IConsumer<ValueTypeT> {
  using ValueType = ValueTypeT;

  explicit FirstContainer(Producer1 p1, Producer2 p2)
      : producer1_(std::move(p1)),
        producer2_(std::move(p2)) {
  }

  void Start(IConsumer<ValueType>* consumer) {
    consumer_ = consumer;
    producer1_.Start(this);
    producer2_.Start(this);
  }

 private:
  void Consume(Output<ValueType> output) noexcept override {
    auto result = std::move(output.result);

    if (result.has_value() && state_.fetch_or(State::Value) != State::Value) {
      consumer_->Complete(std::move(result));
    } else if (state_.fetch_or(State::Error) == State::Error) {
      consumer_->Complete(std::move(result));
    }

    if (producers_counter_.fetch_sub(1) == 1) {
      delete this;
    }
  }

  enum State : uint8_t {
    Init = 0,
    Error = 1,
    Value = 2,
  };

  Producer1 producer1_;
  Producer2 producer2_;
  IConsumer<ValueType>* consumer_;
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
  twist::ed::stdlike::atomic<uint8_t> producers_counter_{2};
};

}  // namespace detail

template <Thunk Producer1, Thunk Producer2,
          typename ValueTypeT = typename Producer1::ValueType>
struct [[nodiscard]] First {
  using ValueType = ValueTypeT;

 public:
  First(Producer1 p1, Producer2 p2) {
    container_ = new detail::FirstContainer(std::move(p1), std::move(p2));
  }

  // Non-copyable
  First(const First&) = delete;

  // Movable
  First(First&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    container_->Start(consumer);
  }

 private:
  detail::FirstContainer<Producer1, Producer2>* container_;
};

}  // namespace exe::futures::thunks
