#include <optional>
#include <tuple>

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/result/make/ok.hpp>
#include <exe/result/make/err.hpp>

#include <exe/futures/model/thunk.hpp>

namespace exe::futures::detail {

template <Thunk Producer1, Thunk Producer2>
struct BothContainer {
  using ValueType1 = typename Producer1::ValueType;
  using ValueType2 = typename Producer2::ValueType;
  using ValueType = std::tuple<ValueType1, ValueType2>;

  explicit BothContainer(Producer1 p1, Producer2 p2)
      : producer1_(std::move(p1)),
        producer2_(std::move(p2)) {
  }

  void Start(IConsumer<ValueType>* consumer) {
    consumer_ = consumer;
    producer1_.Start(this);
    producer2_.Start(this);
  }

 private:
  template <typename T, size_t Index>
  void Consume(Output<T> output) {
    auto result = std::move(output.result);

    if (result.has_value()) {
      std::get<Index - 1>(values_).emplace(std::move(result.value()));
      if (state_.fetch_or(State::Value) == State::Value) {
        auto [v1, v2] = std::move(values_);
        consumer_->Complete(result::Ok(
            std::tuple(std::move(v1.value()), std::move(v2.value()))));
      }
    } else if (state_.fetch_or(State::Error) != State::Error) {
      auto err = std::move(output.result.error());
      consumer_->Complete(result::Err(std::move(err)));
    }

    if (producers_counter_.fetch_sub(1) == 1) {
      delete this;
    }
  }

  template <Thunk Producer, size_t Index>
  struct [[nodiscard]] IndexedThunk final
      : IConsumer<typename Producer::ValueType> {
    using ValueType = typename Producer::ValueType;

   public:
    explicit IndexedThunk(Producer p)
        : producer_(std::move(p)) {
    }

    // Non-copyable
    IndexedThunk(const IndexedThunk&) = delete;

    // Movable
    IndexedThunk(IndexedThunk&&) = default;

    void Start(BothContainer<Producer1, Producer2>* consumer) {
      consumer_ = consumer;
      producer_.Start(this);
    }

   private:
    void Consume(Output<ValueType> output) noexcept override {
      consumer_->Consume<ValueType, Index>(std::move(output));
    }

    Producer producer_;
    BothContainer<Producer1, Producer2>* consumer_;
  };

  enum State : uint8_t {
    Init = 0,
    Error = 1,
    Value = 2,
  };

  IndexedThunk<Producer1, 1> producer1_;
  IndexedThunk<Producer2, 2> producer2_;
  IConsumer<ValueType>* consumer_;

  std::tuple<std::optional<ValueType1>, std::optional<ValueType2>> values_;
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
  twist::ed::stdlike::atomic<uint8_t> producers_counter_{2};
};

}  // namespace exe::futures::detail