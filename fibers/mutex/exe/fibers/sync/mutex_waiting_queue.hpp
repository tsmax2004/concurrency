#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/fibers/core/awaiter.hpp>

// LockFree waiting queue for mutex

namespace exe::fibers {

template <typename T>
struct IntrusiveNode {
 public:
  T* AsItem() {
    return static_cast<T*>(this);
  }

  IntrusiveNode* next_;
};

template <typename T>
class WaitingQueue {
  using IntrusiveNode = IntrusiveNode<T>;

 public:
  bool TryLockLock(IntrusiveNode* node) {
    auto* old = tail_.load();
    do {
      node->next_ = old;
    } while (!tail_.compare_exchange_strong(old, node));

    if (old == nullptr) {
      Pop();
      return true;
    }
    return false;
  }

  T* Pop() {
    if (batch_ == nullptr && !PrepareBatch()) {
      return nullptr;
    }
    auto* obj = batch_->AsItem();
    batch_ = batch_->next_;
    return obj;
  }

 private:
  bool PrepareBatch() {
    auto* free_expected = &empty_;
    if (tail_.compare_exchange_strong(free_expected, nullptr)) {
      return false;
    }

    auto* node = tail_.exchange(&empty_);
    auto* next = node->next_;
    node->next_ = nullptr;
    while (next != nullptr && next != &empty_) {
      auto* tmp_next = next->next_;
      next->next_ = node;
      node = next;
      next = tmp_next;
    }

    batch_ = node;

    return true;
  }

  IntrusiveNode empty_;
  twist::ed::stdlike::atomic<IntrusiveNode*> tail_{nullptr};
  IntrusiveNode* batch_{nullptr};
};

}  // namespace exe::fibers