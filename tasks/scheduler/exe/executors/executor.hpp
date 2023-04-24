#pragma once

#include <exe/executors/task.hpp>
#include <exe/executors/hint.hpp>

namespace exe::executors {

struct IExecutor {
  virtual ~IExecutor() = default;

  // TODO: Support scheduler hints
  virtual void Submit(IntrusiveTask* task) = 0;
};

}  // namespace exe::executors
