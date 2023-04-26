#include <exe/fibers/core/handle.hpp>

#include <exe/fibers/core/fiber.hpp>

#include <wheels/core/assert.hpp>

#include <utility>

namespace exe::fibers {

Fiber* FiberHandle::Release() {
  WHEELS_ASSERT(IsValid(), "Invalid fiber handle");
  return std::exchange(fiber_, nullptr);
}

void FiberHandle::Schedule() {
  Schedule(executors::SchedulerHint::UpToYou);
}

void FiberHandle::Switch() {
  Release()->Switch();
}

void FiberHandle::Schedule(executors::SchedulerHint hint) {
  Release()->Schedule(hint);
}

}  // namespace exe::fibers
