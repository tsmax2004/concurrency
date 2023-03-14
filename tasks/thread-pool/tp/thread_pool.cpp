#include <tp/thread_pool.hpp>

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/panic.hpp>

namespace tp {

ThreadPool::ThreadPool(size_t num_workers)
    : num_workers_(num_workers) {
}

void ThreadPool::Start() {
  for (size_t i = 0; i < num_workers_; ++i) {
    workers_.emplace_back([this] {
      Worker();
    });
  }
}

ThreadPool::~ThreadPool() {
  WHEELS_ASSERT(is_stopped_, "Thread pool wasn't stopped");
}

void ThreadPool::Submit(Task task) {
  tasks_wg_.Add(1);
  task_queue_.Put(std::move(task));
}

ThreadPool* ThreadPool::Current() {
  return pool;
}

void ThreadPool::WaitIdle() {
  tasks_wg_.Wait();
}

void ThreadPool::Stop() {
  is_stopped_.store(1);
  task_queue_.Close();

  for (auto& worker : workers_) {
    worker.join();
  }
}

void ThreadPool::Worker() {
  pool = this;

  while (is_stopped_.load() == 0) {
    if (auto task = task_queue_.Take()) {
      task.value()();
      tasks_wg_.Done();
    }
  }
}

}  // namespace tp
