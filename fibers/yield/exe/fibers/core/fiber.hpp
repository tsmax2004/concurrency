#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>

#include <exe/coro/core.hpp>
#include <exe/fibers/sched/go.hpp>

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (thread pool)

class Fiber {
  friend void Go(Scheduler&, Routine);
  friend void Go(Routine);

 public:
  void Schedule();

  // Task
  void Run();

  static Fiber* Self();

 private:
  Fiber(Scheduler&, Routine);
  explicit Fiber(Routine);

  Scheduler& scheduler_;
  coro::Coroutine coroutine_;
};

}  // namespace exe::fibers
