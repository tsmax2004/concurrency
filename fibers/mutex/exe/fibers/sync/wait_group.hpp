#pragma once

#include <cstdlib>
#include <exe/fibers/sync/event.hpp>

namespace exe::fibers {

// https://gobyexample.com/waitgroups

class WaitGroup {
 public:
  void Add(size_t count) {
    counter_.fetch_add(count);
  }

  void Done() {
    if (counter_.fetch_sub(1) == 1) {
      all_done_event_.Fire();
    }
  }

  void Wait() {
    if (counter_.load() == 0) {
      return;
    }

    all_done_event_.Wait();
  }

 private:
  twist::ed::stdlike::atomic<size_t> counter_{0};
  Event all_done_event_;
};

}  // namespace exe::fibers
