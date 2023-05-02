#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/futures/types/future.hpp>
#include <exe/futures/make/contract.hpp>

namespace exe::futures {

template <typename T>
struct FirstFinish {
  explicit FirstFinish(Promise<T>&& p)
      : promise_(std::move(p)) {
  }

  void With(Result<T> result) {
    uint8_t prev_state;

    if (result.has_value()) {
      auto p = std::move(promise_);
      prev_state = state_.fetch_or(State::Value);
      if (prev_state != State::Value) {
        std::move(p).Set(std::move(result));
      }
    } else {
      prev_state = state_.fetch_or(State::Error);
      if (prev_state == State::Error) {
        std::move(promise_).Set(std::move(result));
      }
    }

    if (prev_state != State::Init) {
      delete this;
    }
  }

 private:
  enum State : uint8_t {
    Init = 0,
    Error = 1,
    Value = 2,
  };

  Promise<T> promise_;
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
};

template <typename T>
Future<T> First(Future<T> f1, Future<T> f2) {
  auto [f, p] = Contract<T>();
  auto finish = new FirstFinish<T>(std::move(p));

  f1.Via(executors::Inline());
  std::move(f1).Consume([finish](Result<T> result) mutable {
    finish->With(std::move(result));
  });

  f2.Via(executors::Inline());
  std::move(f2).Consume([finish](Result<T> result) mutable {
    finish->With(std::move(result));
  });

  return std::move(f);
}

}  // namespace exe::futures
