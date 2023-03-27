#pragma once

#include <vector>

#include <exe/executors/executor.hpp>
#include <exe/tp/intrusive_queue.hpp>
#include <exe/executors/executor.hpp>
#include <exe/tp/wait_group.hpp>

#include <twist/ed/stdlike/thread.hpp>
#include <twist/ed/local/ptr.hpp>
#include <twist/ed/stdlike/atomic.hpp>

namespace exe::tp {

// Fixed-size pool of worker threads

class ThreadPool : public executors::IExecutor {
  using IntrusiveTask = executors::IntrusiveTask;

 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  // Launches worker threads
  void Start();

  // Schedules task for execution in one of the worker threads
  void Submit(IntrusiveTask*);

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

  // Waits until outstanding work count reaches zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  void Stop();

 private:
  void Worker();

  size_t num_workers_;
  std::vector<twist::ed::stdlike::thread> workers_;

  IntrusiveUnboundedBlockingQueue<IntrusiveTask> task_queue_;
  WaitGroup tasks_wg_;  // not completed tasks counter
  twist::ed::stdlike::atomic<uint32_t> is_stopped_{0};
};

}  // namespace exe::tp
