#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/thunks/combine/seq/or_else.hpp>

namespace exe::futures {

namespace pipe {

template <typename F>
struct [[nodiscard]] OrElse {
  F fun;

  explicit OrElse(F f)
      : fun(std::move(f)) {
  }

  template <SomeFuture InputFuture>
  Future<traits::ValueOf<InputFuture>> auto Pipe(InputFuture future) {
    return thunks::OrElse(std::move(future), std::move(fun));
  }
};

}  // namespace pipe

// Future<T> -> (Error -> Result<T>) -> Future<T>

template <typename F>
auto OrElse(F fun) {
  return pipe::OrElse{std::move(fun)};
}

}  // namespace exe::futures
