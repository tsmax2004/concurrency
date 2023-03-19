#pragma once

// https://github.com/Naios/function2
#include <function2/function2.hpp>

namespace exe::executors::tp::compute {

using Task = fu2::unique_function<void()>;

}  // namespace exe::executors::tp::compute
