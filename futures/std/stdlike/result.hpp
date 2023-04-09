#pragma once

#include <tl/expected.hpp>

namespace stdlike::detail {

// https://github.com/TartanLlama/expected

template <typename T>
using Result = tl::expected<T, std::exception_ptr>;

}  // namespace stdlike::detail
