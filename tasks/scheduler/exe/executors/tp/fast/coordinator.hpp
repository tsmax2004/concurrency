#pragma once

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::executors::tp::fast {

// Coordinates workers (stealing, parking)

class Coordinator {
 public:
  size_t GetCurrentEpoch();

  void Wait(size_t old);
  void WakeOne();
  void WakeAll();

 private:
  twist::ed::stdlike::atomic<uint32_t> epoch_{0};
};

}  // namespace exe::executors::tp::fast
