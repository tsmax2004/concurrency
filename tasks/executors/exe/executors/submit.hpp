#pragma once

#include <exe/executors/executor.hpp>
#include <function2/function2.hpp>

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
void Submit(IExecutor& exe, F&& fun) {
  auto user_fun = new UserFunction<F>(std::forward<F>(fun));
  exe.Submit(user_fun);
}

}  // namespace exe::executors
