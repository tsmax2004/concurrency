#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/result/types/result.hpp>
#include <exe/futures/callback/callback.hpp>
#include <exe/futures/make/contract.hpp>

#include <type_traits>

namespace exe::futures {

namespace pipe {

template <typename F>
struct [[nodiscard]] Map {
  F fun;

  explicit Map(F f)
      : fun(std::move(f)) {
  }

  template <typename T>
  using U = std::invoke_result_t<F, T>;

  template <typename T>
  Future<U<T>> Pipe(Future<T> producer) {
    auto [consumer, promise] = Contract<U<T>>(producer.GetExecutor());

    producer.Consume([promise = std::move(promise),
                      fun = std::move(fun)](Result<T> result) mutable {
      std::move(promise).Set(result.map(std::move(fun)));
    });

    return std::move(consumer);
  }
};

}  // namespace pipe

// Future<T> -> (T -> U) -> Future<U>

template <typename F>
auto Map(F fun) {
  return pipe::Map{std::move(fun)};
}

}  // namespace exe::futures
