#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <stdlike/result.hpp>
#include <stdlike/callback.hpp>

namespace stdlike::detail {

template <typename T>
struct SharedBuffer {
 public:
  void SetValue(T&& obj) {
    result_ = std::forward<T>(obj);
    if (state_.fetch_or(State::Produced) == State::Consumed) {
      Rendezvous();
    }
  }

  void SetException(std::exception_ptr eptr) {
    result_.error() = eptr;
    if (state_.fetch_or(State::Produced) == State::Consumed) {
      Rendezvous();
    }
  }

  void Consume(Callback<T>* callback) {
    callback_ = callback;
    if (state_.fetch_or(State::Consumed) == State::Produced) {
      Rendezvous();
    }
  }

 private:
  void Rendezvous() {
    auto result = std::move(result_);
    auto callback = callback_;
    delete this;

    if (callback != nullptr) {
      callback->operator()(std::move(result));
    }
  }

  enum State : uint8_t {
    Init = 0,
    Consumed = 1,
    Produced = 2,
    Rendezvous_ = Consumed | Produced,
  };

  Result<T> result_{tl::make_unexpected(nullptr)};
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
  Callback<T>* callback_;
};

}  // namespace stdlike::detail