#pragma once

#include <exe/fibers/core/handle.hpp>

namespace exe::fibers {

struct IAwaiter {
  friend Fiber;

 public:
  virtual ~IAwaiter() = default;

 private:
  virtual void Await(FiberHandle) = 0;
};

}  // namespace exe::fibers
