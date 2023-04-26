#include <exe/fibers/sched/yield.hpp>
#include <exe/fibers/sched/suspend.hpp>

#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/awaiter.hpp>

namespace exe::fibers {

struct YieldAwaiter : IAwaiter {
 public:
  bool AwaitSuspend(FiberHandle fiber) override {
    fiber.Schedule(executors::SchedulerHint::ToOtherThread);
    return true;
  }
};

void Yield() {
  YieldAwaiter awaiter;
  Suspend(awaiter);
}

}  // namespace exe::fibers
