#pragma once

#include <exe/futures/syntax/pipe.hpp>

namespace exe::futures {

namespace pipe {

template <typename F>
struct [[nodiscard]] OrElse {
  F fun;

  explicit OrElse(F f)
      : fun(std::move(f)) {
  }

  template <typename T>
  Future<T> Pipe(Future<T> producer) {
    auto [consumer, promise] = Contract<T>(producer.GetExecutor());

    producer.Consume([promise = std::move(promise),
                      fun = std::move(fun)](Result<T> result) mutable {
      if (result.has_value()) {
        std::move(promise).Set(std::move(result));
      } else {
        std::move(promise).Set(fun(std::move(result.error())));
      }
    });

    return std::move(consumer);
  }
};

}  // namespace pipe

// Future<T> -> (Error -> Result<T>) -> Future<T>

template <typename F>
auto OrElse(F fun) {
  return pipe::OrElse{std::move(fun)};
}

}  // namespace exe::futures
