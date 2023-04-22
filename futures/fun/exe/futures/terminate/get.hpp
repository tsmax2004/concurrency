#pragma once

#include <optional>

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <exe/futures/syntax/pipe.hpp>

#include <exe/result/types/result.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Get {
  template <typename T>
  Result<T> Pipe(Future<T> future) {
    std::optional<Result<T>> result;
    twist::ed::stdlike::atomic<uint32_t> is_ready{0};

    future.Via(executors::Inline());
    future.Consume([&](Result<T> f_r) {
      result.emplace(std::move(f_r));

      auto key = twist::ed::PrepareWake(is_ready);
      is_ready.store(1);
      twist::ed::WakeOne(key);
    });

    twist::ed::Wait(is_ready, 0);
    return std::move(result.value());
  }
};

}  // namespace pipe

inline auto Get() {
  return pipe::Get{};
}

}  // namespace exe::futures
