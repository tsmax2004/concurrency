#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
  state_ = std::make_shared<twist::ed::stdlike::atomic<StrandState>>(
      StrandState::Chilling);
  task_stack_ = std::make_shared<LockFreePushStack<Task>>();
}

void Strand::Submit(Task task) {
  bool is_chilling =
      state_->exchange(StrandState::Running) == StrandState::Chilling;
  task_stack_->Push(std::move(task));

  if (is_chilling ||
      state_->exchange(StrandState::Waiting) == StrandState::Chilling) {
    Submit();
  }
}

void Strand::Submit() {
  state_->store(StrandState::Running);
  underlying_executor_.Submit(
      [this, state = state_,
       task_stack = task_stack_->GetReversedStack()]() mutable {
        while (!task_stack.empty()) {
          task_stack.top()();
          task_stack.pop();
        }

        auto expired = StrandState::Running;
        if (!state->compare_exchange_strong(expired, StrandState::Chilling)) {
          Submit();
        }
      });
}

}  // namespace exe::executors
