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
  Future<T> Pipe(Future<T> mapping) {
    auto [mapped, p] = Contract<T>(mapping.GetExecutor());

    mapping.Consume(
        [p = std::move(p), fun = std::move(fun)](Result<T> result) mutable {
          if (result.has_value()) {
            std::move(p).Set(std::move(result));
          } else {
            std::move(p).Set(std::move(fun(std::move(result.error()))));
          }
        });

    return std::move(mapped);
  }
};

}  // namespace pipe

// Future<T> -> (Error -> Result<T>) -> Future<T>

template <typename F>
auto OrElse(F fun) {
  return pipe::OrElse{std::move(fun)};
}

}  // namespace exe::futures
