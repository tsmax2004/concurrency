#include <exe/fibers/sched/yield.hpp>
#include <exe/fibers/sched/suspend.hpp>

#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/awaiter.hpp>

namespace exe::fibers {

struct YieldAwaiter : IAwaiter {
 public:
  bool AwaitReady() override {
    return false;
  }

  bool AwaitSuspend(FiberHandle fiber) override {
    fiber.Schedule();
    return true;
  }

  void AwaitResume() override {
    WHEELS_UNREACHABLE();
  }
};

void Yield() {
  YieldAwaiter awaiter;
  Suspend(awaiter);
}

}  // namespace exe::fibers
