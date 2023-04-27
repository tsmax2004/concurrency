#include <exe/executors/tp/fast/thread_pool.hpp>

namespace exe::executors::tp::fast {

ThreadPool::ThreadPool(size_t threads)
    : threads_(threads),
      coordinator_(threads_) {
  for (size_t i = 0; i < threads; ++i) {
    workers_.emplace_back(*this);
  }
}

void ThreadPool::Start() {
  for (auto& worker : workers_) {
    worker.Start();
  }
}

ThreadPool::~ThreadPool() {
  WHEELS_ASSERT(is_stopped_, "ThreadPool was not stopped!");
}

void ThreadPool::Submit(TaskBase* task) {
  Submit(task, SchedulerHint::UpToYou);
}

void ThreadPool::Submit(exe::executors::IntrusiveTask* task,
                        exe::executors::SchedulerHint hint) {
  if (MyWorker(Worker::Current())) {
    Worker::Current()->Push(task, hint);
  } else {
    global_tasks_.Push(task);
  }

  coordinator_.Notify();
}

void ThreadPool::Stop() {
  is_stopped_ = true;

  for (auto& worker : workers_) {
    worker.Stop();
  }

  coordinator_.WakeAll();

  for (auto& worker : workers_) {
    worker.Join();
  }
}

PoolMetrics ThreadPool::Metrics() const {
  std::abort();
}

ThreadPool* ThreadPool::Current() {
  if (Worker::Current() == nullptr) {
    return nullptr;
  }
  return &Worker::Current()->Host();
}

bool ThreadPool::MyWorker(Worker* worker) {
  return worker != nullptr && &worker->Host() == this;
}

}  // namespace exe::executors::tp::fast
