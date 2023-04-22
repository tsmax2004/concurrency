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
  Future<U<T>> Pipe(Future<T> mapping) {
    auto [mapped, p] = Contract<U<T>>(mapping.GetExecutor());

    mapping.Consume(
        [p = std::move(p), fun = std::move(fun)](Result<T> result) mutable {
          if (result.has_value()) {
            std::move(p).SetValue(std::move(fun(std::move(result.value()))));
          } else {
            std::move(p).SetError(std::move(result.error()));
          }
        });

    return std::move(mapped);
  }
};

}  // namespace pipe

// Future<T> -> (T -> U) -> Future<U>

template <typename F>
auto Map(F fun) {
  return pipe::Map{std::move(fun)};
}

}  // namespace exe::futures
