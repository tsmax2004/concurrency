#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
}

void Strand::Submit(Task task) {
  threads::QueueSpinLock::Guard guard(spin_lock_);
  task_queue_.push(std::move(task));
  Submit(guard);
}

void Strand::Submit() {
  threads::QueueSpinLock::Guard guard(spin_lock_);
  is_submitted_ = false;
  Submit(guard);
}

void Strand::Submit(threads::QueueSpinLock::Guard&) {
  if (is_submitted_ || task_queue_.empty()) {
    return;
  }

  underlying_executor_.Submit([this, batch = std::move(task_queue_)]() mutable {
    while (!batch.empty()) {
      batch.front()();
      batch.pop();
    }
    Submit();
  });

  is_submitted_ = true;
}

}  // namespace exe::executors
