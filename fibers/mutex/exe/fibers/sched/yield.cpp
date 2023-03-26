#include <exe/fibers/sched/yield.hpp>
#include <exe/fibers/sched/suspend.hpp>

namespace exe::fibers {

YieldAwaiter yield_awaiter;

void YieldAwaiter::Await(FiberHandle fiber_handle) {
  fiber_handle.Schedule();
}

void Yield() {
  fibers::Suspend(&yield_awaiter);
}

}  // namespace exe::fibers
