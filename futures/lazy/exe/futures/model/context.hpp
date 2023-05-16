#pragma once

#include <exe/executors/inline.hpp>

namespace exe::futures {

// Represents mutable future state

struct Context {
  Context()
      : exe(&executors::Inline()) {
  }

  executors::IExecutor* exe;
};

}  // namespace exe::futures
