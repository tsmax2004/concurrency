#pragma once

#include <exe/futures/types/future.hpp>

#include <tuple>
#include <memory>

#include <exe/futures/types/future.hpp>
#include <exe/futures/make/contract.hpp>

namespace exe::futures {

template <typename X, typename Y>
struct BothFinish {
  explicit BothFinish(Promise<std::tuple<X, Y>>&& p)
      : promise_(std::move(p)) {
  }

  template <typename T>
  void With(Result<T> result, uint8_t order) {
    uint8_t prev_state;
    if (result.has_value()) {
      if (order == 0) {
        value_1_.emplace(std::move(result.value()));
      } else {
        value_2_.emplace(std::move(result.value()));
      }

      prev_state = state_.fetch_or(State::Value);
      if (prev_state == State::Value) {
        std::move(promise_).SetValue({value_1_.value(), value_2_.value()});
      }
    } else {
      prev_state = state_.fetch_or(State::Error);
      if (prev_state != State::Error) {
        std::move(promise_).SetError(std::move(result.error()));
      }
    }
  }

 private:
  enum State : uint8_t {
    Init = 0,
    Error = 1,
    Value = 2,
  };

  std::optional<X> value_1_;
  std::optional<Y> value_2_;
  Promise<std::tuple<X, Y>> promise_;
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
};

template <typename X, typename Y>
Future<std::tuple<X, Y>> Both(Future<X> f1, Future<Y> f2) {
  auto [f, p] = Contract<std::tuple<X, Y>>();
  auto finish = std::make_shared<BothFinish<X, Y>>(std::move(p));

  f1.Via(executors::Inline());
  std::move(f1).Consume([finish](Result<X> result) mutable {
    finish->With(std::move(result), 0);
  });

  f2.Via(executors::Inline());
  std::move(f2).Consume([finish](Result<Y> result) mutable {
    finish->With(std::move(result), 1);
  });

  return std::move(f);
}

// + variadic All

}  // namespace exe::futures
