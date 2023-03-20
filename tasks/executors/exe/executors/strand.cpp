#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
  state_ = std::make_shared<twist::ed::stdlike::atomic<StrandState>>(
      StrandState::Chilling);
}

void Strand::Submit(Task task) {
  bool is_chilling;
  {
    LockGuard guard(spin_lock_);
    task_queue_.push(std::move(task));
    is_chilling =
        state_->exchange(StrandState::Waiting) == StrandState::Chilling;
  }

  if (is_chilling) {
    Submit();
  }
}

void Strand::Submit() {
  underlying_executor_.Submit(
      [this, state = state_, batch = GetBatch()]() mutable {
        while (!batch.empty()) {
          batch.front()();
          batch.pop();
        }

        auto expired = StrandState::Running;
        if (!state->compare_exchange_strong(expired, StrandState::Chilling)) {
          Submit();
        }
      });
}

std::queue<Task> Strand::GetBatch() {
  LockGuard guard(spin_lock_);
  state_->store(StrandState::Running);
  return std::move(task_queue_);
}

}  // namespace exe::executors
