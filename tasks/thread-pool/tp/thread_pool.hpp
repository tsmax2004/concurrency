#pragma once

#include <vector>

#include <tp/queue.hpp>
#include <tp/task.hpp>
#include <tp/wait_group.hpp>

#include <twist/ed/stdlike/thread.hpp>
#include <twist/ed/local/ptr.hpp>
#include <twist/ed/stdlike/atomic.hpp>

namespace tp {

class ThreadPool;
static twist::ed::ThreadLocalPtr<ThreadPool> pool;

// Fixed-size pool of worker threads

class ThreadPool {
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
  void Submit(Task);

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

  UnboundedBlockingQueue<Task> task_queue_;
  WaitGroup tasks_wg_;    // tasks in queue counter
  WaitGroup workers_wg_;  // running workers
  twist::ed::stdlike::atomic<uint32_t> is_stopped_{0};
};

}  // namespace tp
