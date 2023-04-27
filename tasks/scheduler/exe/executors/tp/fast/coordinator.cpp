#include <exe/executors/tp/fast/coordinator.hpp>

namespace exe::executors::tp::fast {

Coordinator::Coordinator(size_t threads)
    : num_workers_(threads) {
  parked_workers_.reserve(num_workers_);
}

void Coordinator::Notify() {
  auto worker = WorkerToWake();
  if (worker == nullptr) {
    return;
  }
  worker->Wake();
}

Worker* Coordinator::WorkerToWake() {
  if (!ShouldWake()) {
    return nullptr;
  }

  std::lock_guard guard(mutex_);
  if (!ShouldWake()) {
    return nullptr;
  }

  auto worker = parked_workers_.back();
  parked_workers_.pop_back();
  num_unparked_.fetch_add(1);
  num_spinning_.fetch_add(1);

  return worker;
}

void Coordinator::WakeAll() {
  std::lock_guard guard(mutex_);
  num_unparked_.store(num_workers_);
  for (auto worker : parked_workers_) {
    worker->Wake();
  }
}

bool Coordinator::TransitToSpinning() {
  if (2 * num_spinning_.load() >= num_workers_) {
    return false;
  }

  // We don't use sync because it's not critical if optimization fail
  num_spinning_.fetch_add(1);
  return true;
}

bool Coordinator::TransitFromSpinning() {
  return num_spinning_.fetch_sub(1) == 1;
}

bool Coordinator::TransitToParked(Worker* worker, bool is_spinning) {
  std::lock_guard guard(mutex_);

  parked_workers_.push_back(worker);
  num_unparked_.fetch_sub(1);

  if (is_spinning) {
    return num_spinning_.fetch_sub(1) == 1;
  }
  return false;
}

bool Coordinator::ShouldWake() {
  return num_spinning_.load() == 0 && num_unparked_.load() < num_workers_;
}

}  // namespace exe::executors::tp::fast