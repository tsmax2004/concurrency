#pragma once

#include <exe/executors/task.hpp>

namespace exe::executors {

// Executors are to function execution as allocators are to memory allocation

struct IExecutor {
  virtual ~IExecutor() = default;

  virtual void Submit(IntrusiveTask* task) = 0;
};

}  // namespace exe::executors
