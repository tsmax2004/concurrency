#pragma once

#include <exe/executors/executor.hpp>

#include <exe/futures/syntax/pipe.hpp>
#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/thunks/combine/seq/via.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Via {
  executors::IExecutor& executor;

  template <SomeFuture InputFuture>
  Future<traits::ValueOf<InputFuture>> auto Pipe(InputFuture f) {
    return thunks::Via(std::move(f), &executor);
  }
};

}  // namespace pipe

// Future<T> -> Executor -> Future<T>

inline auto Via(executors::IExecutor& executor) {
  return pipe::Via{executor};
}

}  // namespace exe::futures
