#pragma once
#include <exe/fibers/core/awaiter.hpp>

namespace exe::fibers {

struct YieldAwaiter : public IAwaiter {
 public:
  void Await(FiberHandle) override;
};

void Yield();

}  // namespace exe::fibers
