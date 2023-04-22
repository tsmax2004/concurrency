#pragma once

#include <exe/futures/types/future.hpp>
#include <exe/futures/make/contract.hpp>

namespace exe::futures {

template <typename T>
Future<T> Value(T value) {
  auto [f, p] = Contract<T>();
  std::move(p).SetValue(std::forward<T>(value));
  return std::move(f);
}

}  // namespace exe::futures
