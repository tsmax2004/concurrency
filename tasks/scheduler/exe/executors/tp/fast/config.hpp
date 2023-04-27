#pragma once

#include <stddef.h>

namespace exe::executors::tp::fast::config {

#if !defined(TWIST_FAULTY)
static const size_t kLocalQueueCapacity = 256;
static const size_t kLifoMaxStreak = 64;
static const size_t kGlobalQueueGrabFrequency = 61;
#else
static const size_t kLocalQueueCapacity = 17;
static const size_t kLifoMaxStreak = 64;
static const size_t kGlobalQueueGrabFrequency = 61;
#endif

}  // namespace exe::executors::tp::fast::config