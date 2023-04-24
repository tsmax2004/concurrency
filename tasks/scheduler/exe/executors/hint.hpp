#pragma once

namespace exe::executors {

enum class SchedulerHint {
  UpToYou = 1,      // Rely on executor scheduling decision
  Next = 2,         // Use LIFO scheduling
  OtherThread = 3,  // Push task in global queue
};

}  // namespace exe::executors
