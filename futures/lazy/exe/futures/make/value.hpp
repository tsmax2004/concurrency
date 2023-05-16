#pragma once

#include <exe/futures/types/future.hpp>

#include <exe/futures/thunks/make/value.hpp>

namespace exe::futures {

template <typename T>
Future<T> auto Value(T value) {
  return thunks::Value<T>{std::move(value)};
}

}  // namespace exe::futures
