#pragma once

#include <exe/coro/core.hpp>

namespace exe::coro {

// Simple stackful coroutine

class Coroutine;
using SimpleCoroutine = Coroutine;

class Coroutine {
 public:
  explicit Coroutine(Routine routine)
      : stack_(AllocateStack()),
        core_(std::move(routine), stack_.MutView()) {
  }

  void Resume() {
    core_.Resume();
  }

  void Suspend() {
    core_.Suspend();
  }

  bool IsCompleted() {
    return core_.IsCompleted();
  }

 private:
  static sure::Stack AllocateStack() {
    static const size_t kDefaultStackSize = 64 * 1024;  // 64 kB
    return sure::Stack::AllocateBytes(kDefaultStackSize);
  }

 private:
  sure::Stack stack_;
  CoroutineCore core_;
};

}  // namespace exe::coro
