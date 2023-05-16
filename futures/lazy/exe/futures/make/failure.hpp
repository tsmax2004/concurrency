#pragma once

#include <exe/futures/types/future.hpp>

#include <exe/futures/thunks/make/failure.hpp>

#include <exe/result/types/error.hpp>

namespace exe::futures {

template <typename T>
Future<T> auto Failure(Error with) {
  return thunks::Failure<T>{std::move(with)};
}

}  // namespace exe::futures
