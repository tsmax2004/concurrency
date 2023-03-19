#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

namespace exe::executors::tp::compute {

class WaitGroup {
 public:
  void Add(size_t num) {
    std::lock_guard guard(mutex_);
    counter_ += num;
  }

  void Done() {
    std::lock_guard guard(mutex_);
    if (--counter_ == 0) {
      is_all_done_.notify_all();
    }
  }

  void Wait() {
    std::unique_lock guard(mutex_);
    while (counter_ > 0) {
      is_all_done_.wait(guard);
    }
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable is_all_done_;
  int counter_{0};
};

}  // namespace exe::executors::tp::compute