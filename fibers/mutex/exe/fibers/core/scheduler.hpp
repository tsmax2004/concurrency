#pragma once

#include <exe/tp/thread_pool.hpp>

namespace exe::fibers {

using Scheduler = executors::tp::ThreadPool;

}  // namespace exe::fibers
