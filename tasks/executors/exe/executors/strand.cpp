#include <exe/executors/strand.hpp>

namespace exe::executors {

bool alive;

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
  alive = true;
  state_ = std::make_shared<twist::ed::stdlike::atomic<StrandState>>(
      StrandState::Chilling);
}

void Strand::Submit(Task task) {
  PushTask(std::move(task));

  if (state_->exchange(StrandState::Waiting) == StrandState::Chilling) {
    Submit();
  }
}

void Strand::Submit() {
  state_->store(StrandState::Running);
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

void Strand::PushTask(Task task) {
  threads::QueueSpinLock::Guard guard(spin_lock_);
  task_queue_.push(std::move(task));
}

std::queue<Task> Strand::GetBatch() {
  threads::QueueSpinLock::Guard guard(spin_lock_);
  return std::move(task_queue_);
}

}  // namespace exe::executors
