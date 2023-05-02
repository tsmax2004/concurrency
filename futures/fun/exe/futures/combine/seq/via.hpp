#pragma once

#include <exe/executors/executor.hpp>

#include <exe/futures/syntax/pipe.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Via {
  executors::IExecutor& executor;

  Via(executors::IExecutor& exe)
      : executor(exe) {
  }

  template <typename T>
  Future<T> Pipe(Future<T> f) {
    f.Via(executor);
    return std::move(f);
  }
};

}  // namespace pipe

// Future<T> -> Executor -> Future<T>

inline auto Via(executors::IExecutor& executor) {
  return pipe::Via{executor};
}

}  // namespace exe::futures
