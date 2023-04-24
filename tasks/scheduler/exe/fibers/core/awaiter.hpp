#pragma once

#include <exe/fibers/core/handle.hpp>

namespace exe::fibers {

struct IAwaiter {
 public:
  virtual ~IAwaiter() = default;

  virtual bool AwaitSuspend(FiberHandle) = 0;
};

}  // namespace exe::fibers
