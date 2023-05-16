#include <optional>

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/futures/state/shared_state.hpp>

namespace exe::futures::detail {

template <typename ValueTypeT>
struct SimpleSharedState : detail::SharedState<ValueTypeT> {
  using ValueType = ValueTypeT;

 public:
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

  IConsumer<ValueType>* consumer_;
  std::optional<Output<ValueType>> output_;
  twist::ed::stdlike::atomic<uint32_t> state_{State::Init};
};

}  // namespace exe::futures::detail