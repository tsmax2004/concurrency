#include <optional>

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/futures/containers/container.hpp>

namespace exe::futures::detail {

template <SomeFuture Producer>
struct EagerContainer final : detail::Container<typename Producer::ValueType>,
                              IConsumer<typename Producer::ValueType> {
  using ValueType = typename Producer::ValueType;

 public:
  explicit EagerContainer(Producer p)
      : producer_(std::move(p)) {
    producer_.Start(this);  // force start
  }

  void Start(IConsumer<ValueType>* consumer) override {
    consumer_ = consumer;
    if (state_.fetch_or(State::Consumed) == State::Produced) {
      Rendezvous();
    }
  }

 private:
  void Consume(Output<ValueType> output) noexcept override {
    output_.emplace(std::move(output));
    if (state_.fetch_or(State::Produced) == State::Consumed) {
      Rendezvous();
    }
  }

  void Rendezvous() {
    consumer_->Complete(std::move(*output_));
    delete this;
  }

  enum State : uint8_t {
    Init = 0,
    Consumed = 1,
    Produced = 2,
    Rendezvous_ = Consumed | Produced,
  };

  Producer producer_;
  IConsumer<ValueType>* consumer_;
  std::optional<Output<ValueType>> output_;
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
};

}  // namespace exe::futures::detail