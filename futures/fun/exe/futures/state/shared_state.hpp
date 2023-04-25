#pragma once

#include <optional>

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/result/types/result.hpp>
#include <exe/futures/callback/callback.hpp>

#include <exe/executors/executor.hpp>
#include <exe/executors/inline.hpp>
#include <exe/executors/submit.hpp>

namespace exe::futures::detail {

template <typename T>
struct SharedBuffer : executors::IntrusiveTask {
 public:
  explicit SharedBuffer(executors::IExecutor& exe)
      : executor_(&exe) {
  }

  void Set(Result<T> result) {
    result_.emplace(std::move(result));
    Produce();
  }

  void SetValue(T&& obj) {
    result_.emplace(std::forward<T>(obj));
    Produce();
  }

  void SetException(Error error) {
    result_.emplace(tl::make_unexpected(std::move(error)));
    Produce();
  }

  void Consume(Callback<T> callback) {
    callback_ = std::move(callback);
    if (state_.fetch_or(State::Consumed) == State::Produced) {
      Rendezvous();
    }
  }

  void Via(executors::IExecutor& exe) {
    executor_ = &exe;
  }

  executors::IExecutor& GetExecutor() {
    return *executor_;
  }

 private:
  void Produce() {
    if (state_.fetch_or(State::Produced) == State::Consumed) {
      Rendezvous();
    }
  }

  void Run() noexcept override {
    callback_(std::move(result_.value()));
    delete this;
  }

  void Rendezvous() {
    executor_->Submit(this);
  }

  enum State : uint8_t {
    Init = 0,
    Consumed = 1,
    Produced = 2,
    Rendezvous_ = Consumed | Produced,
  };

  executors::IExecutor* executor_;
  std::optional<Result<T>> result_;
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
  Callback<T> callback_;
};

}  // namespace exe::futures::detail