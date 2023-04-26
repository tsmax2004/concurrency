#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> current_fiber;

void Fiber::Suspend(IAwaiter& awaiter) {
  awaiter_ = &awaiter;
  coroutine_.Suspend();
}

void Fiber::Schedule() {
  Schedule(executors::SchedulerHint::UpToYou);
}

void Fiber::Schedule(executors::SchedulerHint hint) {
  scheduler_.Submit(this, hint);
}

void Fiber::Switch() {
  Run();
}

void Fiber::Run() noexcept {
  Fiber* prev_fiber = current_fiber;

  current_fiber = this;
  coroutine_.Resume();
  current_fiber = prev_fiber;

  if (coroutine_.IsCompleted()) {
    delete this;
    return;
  }

  if (awaiter_->AwaitSuspend(FiberHandle(this))) {
    return;
  }
  Run();
}

Fiber* Fiber::Self() {
  return current_fiber;
}

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(scheduler),
      coroutine_(std::move(routine)) {
}

}  // namespace exe::fibers
