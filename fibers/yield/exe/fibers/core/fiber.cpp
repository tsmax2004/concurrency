#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> current_fiber;

void Fiber::Schedule() {
  scheduler_.Submit([&] {
    Run();
    if (!coroutine_.IsCompleted()) {
      Schedule();
    }
  });
}

void Fiber::Run() {
  Fiber* previous_fiber = current_fiber;
  current_fiber = this;

  coroutine_.Resume();

  current_fiber = previous_fiber;
}

Fiber* Fiber::Self() {
  return nullptr;  // Not implemented
}

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(scheduler),
      coroutine_(std::move(routine)) {
}

Fiber::Fiber(Routine routine)
    : scheduler_(current_fiber->scheduler_),
      coroutine_(std::move(routine)) {
}

}  // namespace exe::fibers
