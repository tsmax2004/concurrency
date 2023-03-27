#pragma once

#include <exe/fibers/core/fwd.hpp>
#include <wheels/intrusive/list.hpp>

namespace exe::fibers {

// Lightweight non-owning handle to a _suspended_ fiber

class FiberHandle : public wheels::IntrusiveListNode<FiberHandle> {
  friend class Fiber;

 public:
  FiberHandle()
      : FiberHandle(nullptr) {
  }

  static FiberHandle Invalid() {
    return FiberHandle(nullptr);
  }

  bool IsValid() const {
    return fiber_ != nullptr;
  }

  // Schedule to the associated scheduler
  void Schedule();

  // Switch to this fiber immediately
  // For symmetric transfer
  void Switch();

 private:
  explicit FiberHandle(Fiber* fiber)
      : fiber_(fiber) {
  }

  Fiber* Release();

  void Acquire(Fiber*);

 private:
  Fiber* fiber_;
};

}  // namespace exe::fibers
