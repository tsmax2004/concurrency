#pragma once

#include <vector>

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/stdlike/mutex.hpp>

#include <exe/executors/tp/fast/worker.hpp>

namespace exe::executors::tp::fast {

// Coordinates workers (stealing, parking)
// Source:
// https://github.com/tokio-rs/tokio/blob/a86c0522183779cd890afc74791cd0e3ec2f31bb/tokio/src/runtime/scheduler/multi_thread/idle.rs

class Coordinator {
 public:
  explicit Coordinator(size_t threads)
      : num_workers_(threads) {
    parked_workers_.reserve(num_workers_);
  }

  // Wake parked worker if there is no spinning worker
  void Notify() {
    auto worker = WorkerToWake();
    if (worker == nullptr) {
      return;
    }
    worker->Wake();
  }

  // Return worker that can be unparked
  // (return nullptr if there is no parked workers or there is spinning worker)
  Worker* WorkerToWake() {
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

  // Unpark all workers, need in Stop()
  void WakeAll() {
    std::lock_guard guard(mutex_);
    num_unparked_.store(num_workers_);
    for (auto worker : parked_workers_) {
      worker->Wake();
    }
  }

  // Call when worker is starting stealing
  // Return false if it shouldn't steal because of optimization
  // (a lot of workers shouldn't steal at the same time)
  bool TransitToSpinning() {
    if (2 * num_spinning_.load() >= num_workers_) {
      return false;
    }

    // We don't use sync because it's not critical if optimization fail
    num_spinning_.fetch_add(1);
    return true;
  }

  // Call when worker stole a work
  // Return true if this worker is last spinning
  // (in this case it can call Notify to unpark worker)
  bool TransitFromSpinning() {
    return num_spinning_.fetch_sub(1) == 1;
  }

  // Call when worker didn't find work
  // Return true if this worker is last spinning, and
  // he needs to check all queues again
  bool TransitToParked(Worker* worker, bool is_spinning) {
    std::lock_guard guard(mutex_);

    parked_workers_.push_back(worker);
    num_unparked_.fetch_sub(1);

    if (is_spinning) {
      return num_spinning_.fetch_sub(1) == 1;
    }
    return false;
  }

 private:
  bool ShouldWake() {
    return num_spinning_.load() == 0 && num_unparked_.load() < num_workers_;
  }

  const size_t num_workers_;

  std::vector<Worker*> parked_workers_;  // guarded by mutex
  twist::ed::stdlike::mutex mutex_;

  twist::ed::stdlike::atomic<size_t> num_unparked_{num_workers_};
  twist::ed::stdlike::atomic<size_t> num_spinning_{0};
};

}  // namespace exe::executors::tp::fast
