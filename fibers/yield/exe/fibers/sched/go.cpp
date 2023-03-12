#include <exe/fibers/sched/go.hpp>
#include <exe/fibers/core/fiber.hpp>

namespace exe::fibers {

void Go(Scheduler& scheduler, Routine routine) {
  auto fiber = new Fiber(scheduler, std::move(routine));
  fiber->Schedule();
}

void Go(Routine routine) {
  auto fiber = new Fiber(std::move(routine));
  fiber->Schedule();
}

}  // namespace exe::fibers
