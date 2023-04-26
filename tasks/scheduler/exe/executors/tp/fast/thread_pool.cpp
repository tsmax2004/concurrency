#include <exe/executors/tp/fast/thread_pool.hpp>

namespace exe::executors::tp::fast {

ThreadPool::ThreadPool(size_t threads)
    : threads_(threads),
      coordinator_(threads_) {
  for (size_t i = 0; i < threads; ++i) {
    workers_.emplace_back(*this, i);
  }
}

void ThreadPool::Start() {
  for (auto& worker : workers_) {
    worker.Start();
  }
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::Submit(TaskBase* task) {
  task_counter_.Add(1);
  if (MyWorker(Worker::Current())) {
    Worker::Current()->Push(task, SchedulerHint::UpToYou);
  } else {
    global_tasks_.Push(task);
  }

  coordinator_.Notify();
}

void ThreadPool::WaitIdle() {
  task_counter_.Wait();
}

void ThreadPool::Stop() {
  is_stopped_.store(true);
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
