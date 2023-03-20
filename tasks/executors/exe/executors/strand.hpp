#pragma once

#include <queue>
#include <memory>

#include <exe/executors/executor.hpp>
#include <exe/threads/spinlock.hpp>
#include <exe/executors/queue.hpp>

namespace exe::executors {

// Strand / serial executor / asynchronous mutex

class Strand : public IExecutor {
  using LockGuard = threads::QueueSpinLock::Guard;

 public:
  explicit Strand(IExecutor& underlying);

  // Non-copyable
  Strand(const Strand&) = delete;
  Strand& operator=(const Strand&) = delete;

  // Non-movable
  Strand(Strand&&) = delete;
  Strand& operator=(Strand&&) = delete;

  // IExecutor
  void Submit(Task cs) override;

 private:
  void Submit();

  IExecutor& underlying_executor_;
  LockFreePushStack<Task> task_stack_;

  std::shared_ptr<twist::ed::stdlike::atomic<size_t>> submitted_cnt_;
};

}  // namespace exe::executors
