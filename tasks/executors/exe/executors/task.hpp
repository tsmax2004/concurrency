#pragma once

#include <function2/function2.hpp>
#include <wheels/intrusive/list.hpp>

namespace exe::executors {

// Consider intrusive tasks
using Task = fu2::unique_function<void()>;

struct TaskBase;
using IntrusiveTask = TaskBase;

struct ITask {
  virtual ~ITask() = default;

  virtual void Run() noexcept = 0;
};

struct TaskBase : ITask,
                  wheels::IntrusiveListNode<TaskBase> {
  //
};

template <typename F>
struct UserFunction : TaskBase {
 public:
  explicit UserFunction(F&& fun)
      : fun_(std::forward<F>(fun)) {
  }

  void Run() noexcept override {
    fun_();
    delete this;
  }

 private:
  F fun_;
};

}  // namespace exe::executors
