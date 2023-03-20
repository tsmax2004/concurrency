#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
  state_ = std::make_shared<twist::ed::stdlike::atomic<StrandState>>(
      StrandState::Chilling);
  task_queue_ = std::make_shared<StrandLockFreeQueue<Task>>();
}

void Strand::Submit(Task task) {
  bool was_chilling =
      state_->exchange(StrandState::Waiting) == StrandState::Chilling;
  task_queue_->Push(std::move(task));
  if (was_chilling ||
      state_->exchange(StrandState::Waiting) == StrandState::Chilling) {
    Submit();
  }
}

void Strand::Submit() {
  underlying_executor_.Submit(
      [this, state = state_, queue = task_queue_]() mutable {
        state_->store(StrandState::Running);
        queue->PreparePop();
        while (!queue->PopEmpty()) {
          queue->Pop()();
        }

        auto expired = StrandState::Running;
        if (!state->compare_exchange_strong(expired, StrandState::Chilling) ||
            !queue->PushEmpty()) {
          Submit();
        }
      });
}

}  // namespace exe::executors
