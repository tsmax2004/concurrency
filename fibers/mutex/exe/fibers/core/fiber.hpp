#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>
#include <exe/fibers/core/awaiter.hpp>

#include <exe/coro/core.hpp>

#include <exe/executors/task.hpp>

#include <exe/fibers/sched/go.hpp>

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (executor)

class Fiber : public executors::IntrusiveTask,
              public wheels::IntrusiveListNode<Fiber> {
  friend void Go(Scheduler&, Routine);
  friend void Go(Routine);

 public:
  void Suspend(IAwaiter*);

  void Schedule();
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
