#pragma once

#include <queue>
#include <memory>

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
  enum class StrandState {
    Chilling = 0,  // nobody run, no tasks
    Running = 1,   // strand is running, no new tasks
    Waiting = 2,   // strand is running, there are new tasks
  };

  void Submit();
  void Submit(threads::QueueSpinLock::Guard&);

  IExecutor& underlying_executor_;

  threads::QueueSpinLock spin_lock_;
  std::queue<Task> task_queue_;

  std::shared_ptr<twist::ed::stdlike::atomic<StrandState>> state_;
};

}  // namespace exe::executors
