#include <twist/ed/stdlike/atomic.hpp>

template <typename T>
struct IntrusiveQueueNode {
 public:
  T* AsItem() {
    return static_cast<T*>(this);
  }

  IntrusiveQueueNode<T>* next_;
};

template <typename T>
class WaitingQueue {
 public:
  bool TryLockPush(IntrusiveQueueNode<T>* node) {
    IntrusiveQueueNode<T>* next = tail_.load();
    node->next_ = next;
    while (!tail_.compare_exchange_strong(next, node)) {
      node->next_ = next;
    }

    return next == &free_;
  }

  T* Pop() {
    if (!PrepareHead()) {
      return nullptr;
    }

    IntrusiveQueueNode<T>* ret = head_;
    head_ = head_->next_;
    return ret->AsItem();
  }

 private:
  bool PrepareHead() {
    if (head_ != nullptr) {
      return true;
    }
    if (TryOpen()) {
      return false;
    }

    WaitingQueue<T> tmp;
    IntrusiveQueueNode<T>* node = tail_.exchange(&locked_);
    while (node != &free_ && node != &locked_ && node != nullptr) {
      IntrusiveQueueNode<T>* tmp_next = node->next_;
      tmp.PushOnHead(node);
      node = tmp_next;
    }

    head_ = tmp.head_;
    return true;
  }

  void PushOnHead(IntrusiveQueueNode<T>* node) {
    IntrusiveQueueNode<T>* next = head_;
    head_ = node;
    head_->next_ = next;
  }

  bool TryOpen() {
    IntrusiveQueueNode<T>* expected = &locked_;
    return tail_.compare_exchange_strong(expected, &free_);
  }

  IntrusiveQueueNode<T> free_;
  IntrusiveQueueNode<T> locked_;

  IntrusiveQueueNode<T>* head_{nullptr};
  twist::ed::stdlike::atomic<IntrusiveQueueNode<T>*> tail_{&free_};
};