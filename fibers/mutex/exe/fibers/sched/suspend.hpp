#pragma once

#include <exe/fibers/core/awaiter.hpp>

namespace exe::fibers {

void Suspend(IAwaiter*);

}  // namespace exe::fibers
