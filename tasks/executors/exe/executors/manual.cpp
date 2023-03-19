#include <exe/executors/manual.hpp>

namespace exe::executors {

void ManualExecutor::Submit(Task task) {
  task_queue_.push(std::move(task));
}

// Run tasks

size_t ManualExecutor::RunAtMost(size_t limit) {
  size_t completed = 0;
  while (NonEmpty() && completed < limit) {
    task_queue_.front()();
    task_queue_.pop();
    ++completed;
  }

  return completed;
}

size_t ManualExecutor::Drain() {
  return RunAtMost(UINT64_MAX);
}

}  // namespace exe::executors
