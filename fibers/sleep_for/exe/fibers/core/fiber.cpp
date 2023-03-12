#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

#include <asio/post.hpp>
#include <asio/defer.hpp>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> current_fiber;

void Fiber::Schedule() {
  asio::post(scheduler_, [&] {
    if (state_ == FibetState::Running) {
      asio::defer(scheduler_, [&] {
        Schedule();
      });
    } else {
      Run();
    }
  });
}

void Fiber::Run() {
  Fiber* previous_fiber = current_fiber;
  current_fiber = this;
  state_ = FibetState::Running;

  coroutine_.Resume();

  state_ = FibetState::Suspended;
  current_fiber = previous_fiber;

  if (coroutine_.IsCompleted()) {
    delete this;
  }
}

Fiber* Fiber::Self() {
  return current_fiber;
}

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(scheduler),
      coroutine_(std::move(routine)) {
}

}  // namespace exe::fibers
