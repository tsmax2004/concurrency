#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <cstdint>
#include <variant>

namespace stdlike::detail {

enum BuffetState : uint32_t {
  NOT_READY = 0,
  OBJECT = 1,
  EXCEPTION = 2,
};

template <typename T>
struct Buffer {
  Buffer() = default;

  twist::ed::stdlike::mutex mutex;
  twist::ed::stdlike::condition_variable is_ready_cv;
  std::variant<std::exception_ptr, T> value;
  BuffetState state_{0};
};

template <>
struct Buffer<std::exception_ptr> {
  Buffer() = default;

  twist::ed::stdlike::mutex mutex;
  twist::ed::stdlike::condition_variable is_ready_cv;
  std::variant<std::exception_ptr> value;
  BuffetState state_{0};
};

}  // namespace stdlike::detail