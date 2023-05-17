#pragma once

#include <exe/futures/types/future.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/thunks/combine/par/first.hpp>

#include <type_traits>

namespace exe::futures {

template <SomeFuture LeftFuture, SomeFuture RightFuture>
BoxedFuture<traits::ValueOf<LeftFuture>> First(LeftFuture f1, RightFuture f2) {
  static_assert(std::is_same_v<traits::ValueOf<LeftFuture>,
                               traits::ValueOf<RightFuture>>);

  return thunks::First(std::move(f1), std::move(f2));
}

}  // namespace exe::futures
