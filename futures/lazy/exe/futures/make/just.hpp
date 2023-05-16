#pragma once

#include <exe/futures/types/future.hpp>
#include <exe/result/types/unit.hpp>

#include <exe/futures/thunks/make/just.hpp>

namespace exe::futures {

inline Future<Unit> auto Just() {
  return thunks::Just{};
}

}  // namespace exe::futures
