#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/make/contract.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Flatten {
  template <typename T>
  Future<T> Pipe(Future<Future<T>> ff) {
    auto [f, p] = Contract<T>(ff.GetExecutor());

    ff.Consume([p = std::move(p)](Result<Future<T>> rf) mutable {
      if (rf.has_value()) {
        rf.value().Consume([p = std::move(p)](Result<T> r) mutable {
          std::move(p).Set(std::move(r));
        });
      } else {
        std::move(p).SetError(std::move(rf.error()));
      }
    });

    return std::move(f);
  }
};

}  // namespace pipe

// Future<Future<T>> -> Future<T>

inline auto Flatten() {
  return pipe::Flatten{};
}

}  // namespace exe::futures
