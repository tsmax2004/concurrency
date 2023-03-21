#pragma once

#include <queue>

#include <exe/executors/executor.hpp>

namespace exe::executors {

// Single-threaded task queue

class ManualExecutor : public IExecutor {
  using IntrusiveList = wheels::IntrusiveList<TaskBase>;

 public:
  ManualExecutor() = default;

  // Non-copyable
  ManualExecutor(const ManualExecutor&) = delete;
  ManualExecutor& operator=(const ManualExecutor&) = delete;

  // Non-movable
  ManualExecutor(ManualExecutor&&) = delete;
  ManualExecutor& operator=(ManualExecutor&&) = delete;

  // IExecutor
  void Submit(TaskBase*) override;

  // Run tasks

  // Run at most `limit` tasks from queue
  // Returns number of completed tasks
  size_t RunAtMost(size_t limit);

  // Run next task if queue is not empty
  bool RunNext() {
    return RunAtMost(1) == 1;
  }

  // Run tasks until queue is empty
  // Returns number of completed tasks
  // Post-condition: IsEmpty() == true
  size_t Drain();

  size_t TaskCount() const {
    return task_list_.Size();
  }

  bool IsEmpty() const {
    return task_list_.IsEmpty();
  }

  bool NonEmpty() const {
    return !IsEmpty();
  }

 private:
  IntrusiveList task_list_;
};

}  // namespace exe::executors
