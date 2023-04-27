#pragma once

namespace exe::executors::tp::fast::config {

#if !defined(TWIST_FAULTY)
static const size_t kLocalQueueCapacity = 256;
static const size_t kLifoMaxStreak = 64;
static const size_t kGlobalQueueGrabFrequency = 61;
#else
static const size_t kLocalQueueCapacity = 17;
static const size_t kLifoMaxStreak = 4;
static const size_t kGlobalQueueGrabFrequency = 11;
#endif

}  // namespace exe::executors::tp::fast::config