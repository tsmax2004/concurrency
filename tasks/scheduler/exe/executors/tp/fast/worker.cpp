#include <exe/executors/tp/fast/worker.hpp>
#include <exe/executors/tp/fast/thread_pool.hpp>

#include <twist/ed/local/ptr.hpp>

#include <deque>

namespace exe::executors::tp::fast {

twist::ed::ThreadLocalPtr<Worker> this_worker{nullptr};

Worker::Worker(ThreadPool& host, size_t index)
    : host_(host),
      index_(index) {
}

void Worker::Start() {
  thread_.emplace([this]() {
    this_worker = this;
    Work();
  });
}

void Worker::Stop() {
  is_end_.store(true);
}

void Worker::Join() {
  thread_->join();
}

Worker* Worker::Current() {
  return this_worker;
}

void Worker::Push(TaskBase* task, SchedulerHint /*hint*/) {
  if (PushToLocalQueue(task)) {
    return;
  }

  OffloadTasksToGlobalQueue(task);
}

IntrusiveTask* Worker::PickTask() {
  // Poll in order:
  // * [%61] Global queue
  // * LIFO slot
  // * Local queue
  // * Global queue
  // * Work stealing
  // Then
  //   Park worker

  while (!is_end_.load()) {
    auto epoch = host_.coordinator_.GetCurrentEpoch();
    auto task = TryPickTask();

    if (task != nullptr) {
      return task;
    }

    host_.coordinator_.Wait(epoch);
  }

  return nullptr;
}

void Worker::Work() {
  IntrusiveTask* task;
  while ((task = PickTask()) != nullptr) {
    task->Run();
    host_.task_counter_.Done();
  }
}

bool Worker::PushToLocalQueue(IntrusiveTask* task) {
  return local_tasks_.TryPush(task);
}

void Worker::OffloadTasksToGlobalQueue(IntrusiveTask* overflow) {
  tmp_buf_[0] = overflow;
  auto sz = local_tasks_.Grab(std::span(tmp_buf_ + 1, kLocalQueueCapacity / 2));

  host_.global_tasks_.Offload(std::span(tmp_buf_, sz + 1));
}

IntrusiveTask* Worker::TryPickTask() {
  static const size_t kGlobalQueueGrabFrequency = 61;

  IntrusiveTask* task;

  if (++iter_ % kGlobalQueueGrabFrequency == 0) {
    if ((task = TryGrabTasksFromGlobalQueue()) != nullptr) {
      return task;
    }
  }

  if ((task = local_tasks_.TryPop()) != nullptr) {
    return task;
  }

  if ((task = TryGrabTasksFromGlobalQueue()) != nullptr) {
    return task;
  }

  if ((task = TryStealTasks()) != nullptr) {
    return task;
  }

  return nullptr;
}

IntrusiveTask* Worker::TryStealTasks() {
  static const size_t kTasksToSteal = kLocalQueueCapacity / host_.threads_;

  //  std::mt19937_64 gen(host_.random_());
  size_t stolen_tasks = 0;
  //  std::deque<Worker*> workers;
  for (auto& worker : host_.workers_) {
    //    if (&worker != this) {
    //      workers.push_back(&worker);
    //    }
    if (&worker == this) {
      continue;
    }
    auto sz = worker.local_tasks_.Grab(
        std::span(tmp_buf_ + stolen_tasks, kTasksToSteal - stolen_tasks));
    stolen_tasks += sz;
  }

  //  while (stolen_tasks < kTasksToSteal && !workers.empty()) {
  //    std::uniform_int_distribution<int> uniform_dist(0, workers.size() - 1);
  //    auto i = uniform_dist(gen);
  //
  //    auto sz = workers[i]->local_tasks_.Grab(
  //        std::span(tmp_buf_ + stolen_tasks, kTasksToSteal - stolen_tasks));
  //    stolen_tasks += sz;
  //
  //    workers.erase(workers.begin() + i);
  //  }

  if (stolen_tasks == 0) {
    return nullptr;
  }
  local_tasks_.PushMany(std::span(tmp_buf_ + 1, stolen_tasks - 1));
  host_.coordinator_.WakeOne();
  return tmp_buf_[0];
}

IntrusiveTask* Worker::TryGrabTasksFromGlobalQueue() {
  size_t expected_sz = (kLocalQueueCapacity - local_tasks_.Size()) / 2 + 1;
  auto sz = host_.global_tasks_.Grab(std::span(tmp_buf_, expected_sz),
                                     host_.threads_);

  if (sz == 0) {
    return nullptr;
  }
  local_tasks_.PushMany(std::span(tmp_buf_ + 1, sz - 1));
  host_.coordinator_.WakeOne();
  return tmp_buf_[0];
}

}  // namespace exe::executors::tp::fast
