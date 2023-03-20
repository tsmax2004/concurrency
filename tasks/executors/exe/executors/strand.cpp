#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying)
    : underlying_executor_(underlying) {
  submitted_cnt_ = std::make_shared<Counter>(0);
}

void Strand::Submit(Task task) {
  auto task_num = submitted_cnt_->fetch_add(1);
  task_stack_.Push(std::move(task));
  if (task_num == 0) {
    Submit();
  }
}

void Strand::Submit() {
  underlying_executor_.Submit(
      [this, submitted_cnt = submitted_cnt_,
       task_stack = task_stack_.GetReversedStack()]() mutable {
        size_t tasks_cnt = task_stack.size();
        while (!task_stack.empty()) {
          task_stack.top()();
          task_stack.pop();
        }

        if (submitted_cnt->fetch_sub(tasks_cnt) != tasks_cnt) {
          Submit();
        }
      });
}

}  // namespace exe::executors
