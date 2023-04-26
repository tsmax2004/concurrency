#include <exe/executors/tp/fast/worker.hpp>
#include <exe/executors/tp/fast/thread_pool.hpp>

#include <twist/ed/local/ptr.hpp>

#include <deque>

namespace exe::executors::tp::fast {

twist::ed::ThreadLocalPtr<Worker> this_worker{nullptr};

Worker::Worker(ThreadPool& host)
    : host_(host),
      twister_(host_.random_()) {
}

void Worker::Start() {
  thread_.emplace([this]() {
    this_worker = this;
    Work();
  });
}

void Worker::Work() {
  IntrusiveTask* task;
  while ((task = PickTask()) != nullptr) {
    TransitFromSpinning();

    task->Run();
    host_.task_counter_.Done();
  }
}

void Worker::Park() {
  wakeups_.store(1);

  if (!TransitToParked()) {
    wakeups_.store(0);
    return;
  }

  do {
    if (!host_.is_stopped_.load()) {
      twist::ed::Wait(wakeups_, 1);
    }
  } while (!TransitFromParked());
}

void Worker::Join() {
  thread_->join();
}

void Worker::Wake() {
  auto key = twist::ed::PrepareWake(wakeups_);
  wakeups_.store(0);
  twist::ed::WakeOne(key);
}

Worker* Worker::Current() {
  return this_worker;
}

void Worker::Push(TaskBase* task, SchedulerHint hint) {
  static const size_t kLifoMaxStreak = 64;

  if (hint == SchedulerHint::Next && ++lifo_streak_ < kLifoMaxStreak) {
    PushToLifoSlot(task);
    return;
  }
  lifo_streak_ = 0;

  if (hint == SchedulerHint::OtherThread) {
    host_.global_tasks_.Push(task);
    return;
  }

  if (hint == SchedulerHint::UpToYou && PushToLocalQueue(task)) {
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

  static const size_t kGlobalQueueGrabFrequency = 61;

  while (!host_.is_stopped_.load()) {
    IntrusiveTask* task;

    if (++pick_tick_ % kGlobalQueueGrabFrequency == 0) {
      if ((task = TryGrabTasksFromGlobalQueue()) != nullptr) {
        return task;
      }
    }
    if ((task = TryPickTaskFromLifoSlot()) != nullptr) {
      return task;
    }
    if ((task = TryPickTask()) != nullptr) {
      return task;
    }
    if ((task = TryGrabTasksFromGlobalQueue()) != nullptr) {
      return task;
    }
    if ((task = TryStealTasks()) != nullptr) {
      return task;
    }

    Park();
  }

  return nullptr;
}

bool Worker::CheckBeforePark() {
  if (host_.global_tasks_.HasItems()) {
    return true;
  }
  for (auto& worker : host_.workers_) {
    if (worker.local_tasks_.HasItems()) {
      return true;
    }
  }

  return false;
}

size_t Worker::StealTasks(std::span<TaskBase*> out_buffer) {
  return local_tasks_.Grab(out_buffer);
}

void Worker::PushToLifoSlot(IntrusiveTask* task) {
  if (lifo_slot_ != nullptr) {
    Push(lifo_slot_, SchedulerHint::UpToYou);
  }
  lifo_slot_ = task;
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
  return local_tasks_.TryPop();
}

IntrusiveTask* Worker::TryPickTaskFromLifoSlot() {
  auto ret = lifo_slot_;
  lifo_slot_ = nullptr;
  return ret;
}

IntrusiveTask* Worker::TryStealTasks() {
  static const auto kTaskToSteal = kLocalQueueCapacity / host_.threads_;

  if (!TransitToSpinning()) {
    return nullptr;
  }

  std::uniform_int_distribution<int> uniform_dist(0, host_.threads_ - 1);
  auto start = uniform_dist(twister_);
  size_t stolen = 0;

  for (size_t i = 0; i < host_.threads_ && stolen == 0; ++i) {
    auto& worker = host_.workers_[(start + i) % host_.threads_];
    if (&worker == this) {
      continue;
    }

    stolen = worker.StealTasks(std::span(tmp_buf_, kTaskToSteal));
  }

  if (stolen == 0) {
    return TryGrabTasksFromGlobalQueue();
  }

  local_tasks_.PushMany(std::span(tmp_buf_ + 1, stolen - 1));
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
  return tmp_buf_[0];
}

bool Worker::TransitToSpinning() {
  if (!is_spinning_) {
    is_spinning_ = host_.coordinator_.TransitToSpinning();
  }
  return is_spinning_;
}

void Worker::TransitFromSpinning() {
  if (!is_spinning_) {
    return;
  }

  is_spinning_ = false;
  if (host_.coordinator_.TransitFromSpinning()) {
    host_.coordinator_.Notify();
  }
}

bool Worker::TransitToParked() {
  if (local_tasks_.HasItems()) {
    return false;
  }

  auto is_last_spinning =
      host_.coordinator_.TransitToParked(this, is_spinning_);
  is_spinning_ = false;

  if (is_last_spinning && CheckBeforePark()) {
    host_.coordinator_.Notify();
  }

  return true;
}

bool Worker::TransitFromParked() {
  is_spinning_ = true;
  return true;
}

}  // namespace exe::executors::tp::fast
