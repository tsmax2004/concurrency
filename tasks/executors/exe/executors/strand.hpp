#pragma once

#include <queue>

#include <exe/executors/executor.hpp>
#include <exe/threads/spinlock.hpp>

namespace exe::executors {

// Strand / serial executor / asynchronous mutex

class Strand : public IExecutor {
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
  void Submit(threads::QueueSpinLock::Guard&);
  std::vector<Task> GetBatch();

  threads::QueueSpinLock spin_lock_;
  std::queue<Task> task_queue_;
  IExecutor& underlying_executor_;
  bool is_submitted_{false};
};

}  // namespace exe::executors
