#pragma once

#include <exe/executors/task.hpp>
#include <exe/executors/hint.hpp>

namespace exe::executors {

struct IExecutor {
  virtual ~IExecutor() = default;

  virtual void Submit(IntrusiveTask* task) = 0;
  virtual void Submit(IntrusiveTask* task, SchedulerHint hint) = 0;
};

}  // namespace exe::executors
