#include <exe/fibers/sched/yield.hpp>
#include <exe/coro/core.hpp>

namespace exe::fibers {

void Yield() {
  coro::Coroutine::Suspend();
}

}  // namespace exe::fibers
