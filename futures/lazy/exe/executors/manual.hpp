#pragma once

#include <exe/executors/executor.hpp>

#include <wheels/intrusive/forward_list.hpp>

namespace exe::executors {

// Single-threaded task queue

class ManualExecutor : public IExecutor {
  using IntrusiveList = wheels::IntrusiveForwardList<IntrusiveTask>;

 public:
  ManualExecutor() = default;

  // Non-copyable
  ManualExecutor(const ManualExecutor&) = delete;
  ManualExecutor& operator=(const ManualExecutor&) = delete;

  // Non-movable
  ManualExecutor(ManualExecutor&&) = delete;
  ManualExecutor& operator=(ManualExecutor&&) = delete;

  // IExecutor
  void Submit(IntrusiveTask* task) override {
    task_list_.PushBack(task);
  }

  void Submit(IntrusiveTask* task, SchedulerHint) override {
    Submit(task);
  }

  // Run tasks

  // Run at most `limit` tasks from queue
  // Returns number of completed tasks
  size_t RunAtMost(size_t limit) {
    size_t completed = 0;
    while (NonEmpty() && completed < limit) {
      task_list_.PopFront()->Run();
      ++completed;
    }

    return completed;
  }

  // Run next task if queue is not empty
  bool RunNext() {
    return RunAtMost(1) == 1;
  }

  // Run tasks until queue is empty
  // Returns number of completed tasks
  // Post-condition: IsEmpty() == true
  size_t Drain() {
    return RunAtMost(UINT64_MAX);
  }

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
