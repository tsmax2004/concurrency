#include <optional>

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/futures/model/thunk.hpp>

namespace exe::futures::detail {

template <Thunk Producer1, Thunk Producer2,
          typename ValueTypeT = typename Producer1::ValueType>
struct FirstContainer final : IConsumer<ValueTypeT> {
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

}  // namespace exe::futures::detail