#include <exe/executors/tp/fast/coordinator.hpp>
#include <twist/ed/wait/sys.hpp>
#include <fmt/printf.h>

namespace exe::executors::tp::fast {

//

size_t Coordinator::GetCurrentEpoch() {
  return epoch_.load();
}

void Coordinator::Wait(size_t old) {
  twist::ed::Wait(epoch_, old);
}

void Coordinator::WakeOne() {
  auto key = twist::ed::PrepareWake(epoch_);
  epoch_.fetch_add(1);
  twist::ed::WakeOne(key);
}

void Coordinator::WakeAll() {
  auto key = twist::ed::PrepareWake(epoch_);
  epoch_.fetch_add(1);
  twist::ed::WakeAll(key);
}

}  // namespace exe::executors::tp::fast
