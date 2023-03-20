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
  enum class StrandState {
    Chilling = 0,  // nobody run, no tasks
    Running = 1,   // strand is running
    Waiting = 2,   // there are not-submitted tasks
  };

  void Submit();

  IExecutor& underlying_executor_;
  std::shared_ptr<LockFreePushStack<Task>> task_stack_;
  std::shared_ptr<twist::ed::stdlike::atomic<StrandState>> state_;
};

}  // namespace exe::executors
