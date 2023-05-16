#include <exe/futures/state/shared_state.hpp>

namespace exe::futures::detail {

template <SomeFuture Producer>
struct BoxedSharedState : detail::SharedState<typename Producer::ValueType> {
  using ValueType = typename Producer::ValueType;

 public:
  explicit BoxedSharedState(Producer p)
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