#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/types/eager.hpp>

#include <exe/futures/thunks/combine/seq/eager.hpp>

#include <exe/futures/state/shared_state.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Start {
  template <SomeFuture InputFuture>
  EagerFuture<traits::ValueOf<InputFuture>> Pipe(InputFuture future) {
    auto* shared_state = new detail::EagerSharedState(std::move(future));
    shared_state->Start();

    auto eager_future =
        thunks::Eager<typename InputFuture::ValueType>(shared_state);
    return std::move(eager_future);
  }
};

}  // namespace pipe

// Future<T> -> EagerFuture<T>

inline auto Start() {
  return pipe::Start{};
}

inline auto Force() {
  return pipe::Start{};
}

}  // namespace exe::futures
