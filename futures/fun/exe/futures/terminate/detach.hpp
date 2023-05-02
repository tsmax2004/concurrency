#pragma once

#include <exe/futures/syntax/pipe.hpp>
#include <exe/futures/callback/callback.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Detach {
  template <typename T>
  void Pipe(Future<T> f) {
    f.Via(executors::Inline());
    f.Consume([](Result<T>) {});
  }
};

}  // namespace pipe

inline auto Detach() {
  return pipe::Detach{};
}

}  // namespace exe::futures
