#pragma once

#include <exe/executors/executor.hpp>

namespace exe::executors {

/*
 * Usage:
 *
 * Submit(thread_pool, [] {
 *   fmt::println("Running on thread pool");
 * });
 *
 */

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

template <typename F>
void Submit(IExecutor& exe, F&& fun) {
  auto user_fun = new UserFunction<F>(std::forward<F>(fun));
  exe.Submit(user_fun);
}

}  // namespace exe::executors
