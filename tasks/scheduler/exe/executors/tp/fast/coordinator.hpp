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
  explicit Coordinator(size_t threads);

  // Wake parked worker if there is no spinning worker
  void Notify();

  // Unpark all workers (e.g. for Stop())
  void WakeAll();

  // Call when worker is starting stealing
  // Return false if it shouldn't steal because of optimization
  // (a lot of workers shouldn't steal at the same time)
  bool TransitToSpinning();

  // Call when worker is finishing stealing
  // Return true if this worker is last spinning
  // (in this case it can call Notify to unpark worker)
  bool TransitFromSpinning();

  // Call when worker didn't find work
  // Return true if this worker is last spinning, and
  // he needs to check all queues again
  bool TransitToParked(Worker* worker, bool is_spinning);

 private:
  // Return worker that can be unparked
  // (return nullptr if there is no parked workers or there is spinning worker)
  Worker* WorkerToWake();

  bool ShouldWake();

  const size_t num_workers_;

  std::vector<Worker*> parked_workers_;  // guarded by mutex
  twist::ed::stdlike::mutex mutex_;

  twist::ed::stdlike::atomic<size_t> num_unparked_{num_workers_};
  twist::ed::stdlike::atomic<size_t> num_spinning_{0};
};

}  // namespace exe::executors::tp::fast
