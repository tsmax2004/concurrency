#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

#include <asio/post.hpp>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> current_fiber;

void Fiber::Schedule() {
  asio::post(scheduler_, [&] {
    Run();
  });
}

void Fiber::Run() {
  Fiber* previous_fiber = current_fiber;
  current_fiber = this;

  coroutine_.Resume();

  current_fiber = previous_fiber;

  if (coroutine_.IsCompleted()) {
    delete this;
  } else {
    ScheduleSuspendedRoutine();
  }
}

Fiber* Fiber::Self() {
  return current_fiber;
}

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(scheduler),
      coroutine_(std::move(routine)) {
}

void Fiber::SetSuspendedRoutine(Routine routine) {
  suspended_routine_ = std::move(routine);
}

void Fiber::ScheduleSuspendedRoutine() {
  asio::post(scheduler_, std::move(suspended_routine_));
}

}  // namespace exe::fibers
