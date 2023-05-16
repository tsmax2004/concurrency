#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>

namespace exe::futures::detail {

template <typename ValueType>
struct Container {
 public:
  virtual void Start(IConsumer<ValueType>* consumer) = 0;
};

}  // namespace exe::futures::detail