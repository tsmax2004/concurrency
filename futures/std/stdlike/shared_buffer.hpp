#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <stdlike/result.hpp>

namespace stdlike::detail {

template <typename T>
struct SharedBuffer {
 public:
  void SetValue(T&& obj) {
    *(T*)buffer_ = std::forward<T>(obj);
    if (state_.fetch_or(State::Produce) == State::Consume) {
      state_.notify_one();
    }
  }

  void SetException(std::exception_ptr eptr) {
    eptr_ = eptr;
    if (state_.fetch_or(State::Produce) == State::Consume) {
      state_.notify_one();
    }
  }

  Result<T> Get() {
    state_.fetch_or(State::Consume);
    state_.wait(State::Consume);

    if (eptr_ != nullptr) {
      auto result = Result<T>(tl::make_unexpected(eptr_));
      delete this;
      return Result<T>(tl::make_unexpected(eptr_));
    }

    auto result = Result<T>(std::move(*(T*)buffer_));
    delete this;
    return std::move(result);
  }

 private:
  enum State : uint8_t {
    Init = 0,
    Consume = 1,
    Produce = 2,
    Rendezvous = Consume | Produce,
  };

  char buffer_[sizeof(T)];
  std::exception_ptr eptr_{nullptr};
  twist::ed::stdlike::atomic<uint8_t> state_{State::Init};
};

}  // namespace stdlike::detail