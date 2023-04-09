#pragma once

#include <stdlike/result.hpp>

namespace stdlike::detail {

template <typename T>
struct Callback {
 public:
  virtual void operator()(Result<T>) = 0;
};

}  // namespace stdlike::detail