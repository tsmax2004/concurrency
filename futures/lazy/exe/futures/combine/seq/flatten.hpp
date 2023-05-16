#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/thunks/combine/seq/flatten.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Flatten {
  template <SomeFuture InputFuture>
  auto Pipe(InputFuture future) {
    return thunks::Flatten(std::move(future));
  }
};

}  // namespace pipe

// Future<Future<T>> -> Future<T>

inline auto Flatten() {
  return pipe::Flatten{};
}

}  // namespace exe::futures
