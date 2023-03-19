#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
  state_ = std::make_shared<twist::ed::stdlike::atomic<StrandState>>(
      StrandState::Chilling);
}

void Strand::Submit(Task task) {
  threads::QueueSpinLock::Guard guard(spin_lock_);
  task_queue_.push(std::move(task));

  if (state_->exchange(StrandState::Waiting) == StrandState::Chilling) {
    Submit(guard);
  }
}

void Strand::Submit() {
  threads::QueueSpinLock::Guard guard(spin_lock_);
  Submit(guard);
}

void Strand::Submit(threads::QueueSpinLock::Guard&) {
  state_->store(StrandState::Running);
  underlying_executor_.Submit([this, state = std::shared_ptr(state_),
                               batch = std::move(task_queue_)]() mutable {
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
}  // namespace exe::executors
