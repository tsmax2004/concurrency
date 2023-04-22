#pragma once

#include <function2/function2.hpp>

#include <wheels/intrusive/list.hpp>

namespace exe::executors {

struct TaskBase;
using IntrusiveTask = TaskBase;

struct ITask {
  virtual ~ITask() = default;

  virtual void Run() noexcept = 0;
};

struct TaskBase : ITask,
                  wheels::IntrusiveListNode<TaskBase> {};

}  // namespace exe::executors
