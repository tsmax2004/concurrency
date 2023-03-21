#include <exe/executors/manual.hpp>

namespace exe::executors {

void ManualExecutor::Submit(TaskBase* task) {
  task_list_.PushBack(task);
}

// Run tasks

size_t ManualExecutor::RunAtMost(size_t limit) {
  size_t completed = 0;
  while (NonEmpty() && completed < limit) {
    task_list_.PopFront()->Run();
    ++completed;
  }

  return completed;
}

size_t ManualExecutor::Drain() {
  return RunAtMost(UINT64_MAX);
}

}  // namespace exe::executors
