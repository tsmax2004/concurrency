#pragma once

#include <optional>

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>
#include <exe/fibers/core/handle.hpp>

#include <exe/fibers/sched/go.hpp>

#include <exe/coro/simple.hpp>

#include <exe/executors/executor.hpp>
#include <exe/executors/hint.hpp>

#include <exe/fibers/core/awaiter.hpp>

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (executor)

class Fiber : executors::IntrusiveTask {
  friend void Go(Scheduler&, Routine);
  friend void Go(Routine);

 public:
  void Suspend(IAwaiter&);

  void Schedule();
  void Schedule(executors::SchedulerHint);
  void Switch();

  // Task
  void Run() noexcept override;

  static Fiber* Self();

 private:
  Fiber(Scheduler&, Routine);

  Scheduler& scheduler_;
  coro::Coroutine coroutine_;
  IAwaiter* awaiter_;
};

}  // namespace exe::fibers
