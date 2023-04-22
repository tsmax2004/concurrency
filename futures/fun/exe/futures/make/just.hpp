#pragma once

#include <exe/futures/make/value.hpp>
#include <exe/futures/types/future.hpp>
#include <exe/result/types/unit.hpp>

namespace exe::futures {

inline Future<Unit> Just() {
  return Value(Unit());
}

}  // namespace exe::futures
