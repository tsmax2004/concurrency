#pragma once

#include <exe/executors/task.hpp>
#include <exe/executors/hint.hpp>

#include <exe/executors/tp/fast/queues/work_stealing_queue.hpp>
#include <exe/executors/tp/fast/metrics.hpp>

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/stdlike/thread.hpp>

#include <cstdlib>
#include <optional>
#include <random>
#include <span>

namespace exe::executors::tp::fast {

class ThreadPool;

class Worker {
 private:
#if !defined(TWIST_FAULTY)
  static const size_t kLocalQueueCapacity = 256;
#else
  static const size_t kLocalQueueCapacity = 17;
#endif

 public:
  Worker(ThreadPool& host);

  void Start();
  void Join();

  // Single producer
  void Push(TaskBase*, SchedulerHint);

  // Steal from this worker
  size_t StealTasks(std::span<TaskBase*> out_buffer);

  // Wake parked worker
  void Wake();

  static Worker* Current();

  WorkerMetrics Metrics() const {
    return metrics_;
  }

  ThreadPool& Host() const {
    return host_;
  }

 private:
  // Use in Push
  void PushToLifoSlot(IntrusiveTask* task);
  bool PushToLocalQueue(IntrusiveTask* task);
  void OffloadTasksToGlobalQueue(IntrusiveTask* overflow);

  // Use in TryPickTask
  IntrusiveTask* TryPickTaskFromLifoSlot();
  IntrusiveTask* TryStealTasks();
  IntrusiveTask* TryGrabTasksFromGlobalQueue();

  // Use in PickTask
  TaskBase* TryPickTask();
  bool CheckBeforePark();

  // Or park thread
  TaskBase* PickTask();

  // Run Loop
  void Work();

 private:
  void Park();

  bool TransitToSpinning();
  void TransitFromSpinning();
  bool TransitToParked();
  bool TransitFromParked();

  ThreadPool& host_;

  // Worker thread
  std::optional<twist::ed::stdlike::thread> thread_;

  // Scheduling iteration
  size_t pick_tick_{0};

  // Counter of lifo scheduling in a row
  size_t lifo_streak_{0};

  // Local queue
  WorkStealingQueue<kLocalQueueCapacity> local_tasks_;

  // LIFO slot
  IntrusiveTask* lifo_slot_{nullptr};

  // Deterministic pseudo-randomness for work stealing
  std::mt19937_64 twister_;

  // Parking lot
  twist::ed::stdlike::atomic<uint32_t> wakeups_{0};

  // Temp buffer for tasks
  IntrusiveTask* tmp_buf_[kLocalQueueCapacity];

  // Flag for parking mechanism
  bool is_spinning_{false};

  WorkerMetrics metrics_;
};

}  // namespace exe::executors::tp::fast
