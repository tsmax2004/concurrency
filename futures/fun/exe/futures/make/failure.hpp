#pragma once

#include <exe/futures/make/contract.hpp>
#include <exe/futures/types/future.hpp>
#include <exe/result/types/error.hpp>

namespace exe::futures {

template <typename T>
Future<T> Failure(Error err) {
  auto [f, p] = Contract<T>();
  std::move(p).SetError(std::move(err));
  return std::move(f);
}

}  // namespace exe::futures
