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
  std::variant<T, std::exception_ptr> value;
  bool is_ready{false};
};

}  // namespace stdlike::detail