#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> current_fiber;

void Fiber::Suspend(IAwaiter* awaiter) {
  awaiter_ = awaiter;
  coroutine_.Suspend();
}

void Fiber::Schedule() {
  scheduler_.Submit(this);
}

void Fiber::Switch() {
  coroutine_.Resume();
  if (coroutine_.IsCompleted()) {
    delete this;
  } else {
    awaiter_->Await(FiberHandle(this));
  }
}

void Fiber::Run() noexcept {
  Fiber* prev_fiber = current_fiber;
  current_fiber = this;

  coroutine_.Resume();

  if (coroutine_.IsCompleted()) {
    delete this;
  } else {
    awaiter_->Await(FiberHandle(this));
  }

  current_fiber = prev_fiber;
}

Fiber* Fiber::Self() {
  return current_fiber;
}

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(scheduler),
      coroutine_(std::move(routine)) {
}

}  // namespace exe::fibers
