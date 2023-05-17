#pragma once

#include <exe/futures/types/future.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/thunks/combine/par/both.hpp>

#include <tuple>

namespace exe::futures {

template <SomeFuture LeftFuture, SomeFuture RightFuture>
BoxedFuture<
    std::tuple<traits::ValueOf<LeftFuture>, traits::ValueOf<RightFuture>>>
Both(LeftFuture f1, RightFuture f2) {
  return thunks::Both(std::move(f1), std::move(f2));
}

// + variadic All

}  // namespace exe::futures
