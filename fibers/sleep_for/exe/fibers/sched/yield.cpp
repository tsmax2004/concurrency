#include <exe/fibers/sched/yield.hpp>
#include <exe/fibers/core/fiber.hpp>
#include <asio/defer.hpp>
#include <exe/coro/core.hpp>

namespace exe::fibers {

void Yield() {
  auto fiber = Fiber::Self();
  fiber->SetSuspendedRoutine([&] {
    fiber->Schedule();
  });
  coro::Coroutine::Suspend();
}

}  // namespace exe::fibers
