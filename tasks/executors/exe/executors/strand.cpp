#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
  tasks_cnt_ = std::make_shared<twist::ed::stdlike::atomic<size_t>>(0);
  next_task_num_ = std::make_shared<twist::ed::stdlike::atomic<size_t>>(0);
}

void Strand::Submit(Task task) {
  task_stack_.Push(std::move(task));
  if (next_task_num_->fetch_add(1) == tasks_cnt_->load()) {
    Submit();
  }
}

void Strand::Submit() {
  underlying_executor_.Submit(
      [this, tasks_cnt = tasks_cnt_, next_task_num = next_task_num_,
       task_stack = task_stack_.GetReversedStack()]() mutable {
        while (!task_stack.empty()) {
          task_stack.top()();
          task_stack.pop();
          tasks_cnt_->fetch_add(1);
        }

        if (tasks_cnt_->load() != next_task_num_->load()) {
          Submit();
        }
      });
}

}  // namespace exe::executors
