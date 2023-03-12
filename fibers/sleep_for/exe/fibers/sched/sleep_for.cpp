#include <exe/fibers/sched/sleep_for.hpp>

#include <asio/steady_timer.hpp>

#include <exe/fibers/core/fiber.hpp>

namespace exe::fibers {

void SleepFor(Millis delay) {
  auto fiber = Fiber::Self();

  asio::steady_timer timer{fiber->scheduler_};
  timer.expires_after(delay);
  fiber->SetSuspendedRoutine([&] {
    timer.async_wait([&](std::error_code) {
      fiber->Schedule();
    });
  });

  coro::Coroutine::Suspend();
}

}  // namespace exe::fibers
