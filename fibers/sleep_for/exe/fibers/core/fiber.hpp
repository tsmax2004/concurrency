#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>

#include <exe/coro/core.hpp>

#include <exe/fibers/sched/go.hpp>
#include <exe/fibers/sched/sleep_for.hpp>

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (thread pool)

class Fiber {
  friend void Go(Scheduler& scheduler, Routine routine);
  friend void Go(Routine routine);
  friend void SleepFor(Millis);

 public:
  void Schedule();

  // Task
  void Run();

  static Fiber* Self();

 private:
  enum class FibetState {
    Starting,
    Running,
    Suspended,
  };

  Fiber(Scheduler&, Routine);

  Scheduler& scheduler_;
  coro::Coroutine coroutine_;

  FibetState state_;
};

}  // namespace exe::fibers
