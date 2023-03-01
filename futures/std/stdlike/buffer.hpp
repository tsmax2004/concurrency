#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <cstdint>
#include <variant>

namespace stdlike::detail {

template <typename T>
struct Buffer {
  Buffer() = default;

  twist::ed::stdlike::mutex mutex;
  twist::ed::stdlike::condition_variable is_ready_cv;
  std::variant<std::exception_ptr, T> value;
  bool is_ready{false};
  bool is_exception{false};
};

template <>
struct Buffer<std::exception_ptr> {
  Buffer() = default;

  twist::ed::stdlike::mutex mutex;
  twist::ed::stdlike::condition_variable is_ready_cv;
  std::variant<std::exception_ptr> value;
  bool is_ready{false};
  bool is_exception{false};
};

}  // namespace stdlike::detail