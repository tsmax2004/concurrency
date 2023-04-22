#pragma once

#include <memory>

#include <exe/executors/executor.hpp>
#include <exe/executors/support/intrusive_stack.hpp>

namespace exe::executors {

// Strand / serial executor / asynchronous mutex

class Strand : public IExecutor,
               TaskBase {
  using Counter = twist::ed::stdlike::atomic<size_t>;
  using CounterPtr = std::shared_ptr<twist::ed::stdlike::atomic<size_t>>;
  using TaskStack = support::IntrusiveLockFreePushStack;

 public:
  explicit Strand(IExecutor& underlying);

  // Non-copyable
  Strand(const Strand&) = delete;
  Strand& operator=(const Strand&) = delete;

  // Non-movable
  Strand(Strand&&) = delete;
  Strand& operator=(Strand&&) = delete;

  // IExecutor
  //  void Submit(Task) override;
  void Submit(IntrusiveTask*) override;

 private:
  void Run() noexcept override;

  IExecutor& underlying_executor_;
  TaskStack task_stack_;
  CounterPtr submitted_cnt_;
};

}  // namespace exe::executors
