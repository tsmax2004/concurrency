#pragma once

#include <exe/executors/executor.hpp>

#include <exe/executors/tp/fast/queues/global_queue.hpp>
#include <exe/executors/tp/fast/worker.hpp>
#include <exe/executors/tp/fast/coordinator.hpp>
#include <exe/executors/tp/fast/metrics.hpp>
#include <exe/executors/tp/fast/task_counter.hpp>

// random_device
#include <twist/ed/stdlike/random.hpp>

#include <deque>

namespace exe::executors::tp::fast {

// Scalable work-stealing scheduler for short-lived tasks

class ThreadPool : public IExecutor {
  friend class Worker;

 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  void Start();

  // IExecutor
  void Submit(IntrusiveTask*) override;

  void WaitIdle();

  void Stop();

  // After Stop
  PoolMetrics Metrics() const;

  static ThreadPool* Current();

 private:
  bool MyWorker(Worker*);

  const size_t threads_;
  std::deque<Worker> workers_;
  TaskCounter task_counter_;
  Coordinator coordinator_;
  GlobalQueue global_tasks_;
  twist::ed::stdlike::random_device random_;
};

}  // namespace exe::executors::tp::fast
