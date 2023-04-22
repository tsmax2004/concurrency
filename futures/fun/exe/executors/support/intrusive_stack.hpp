#include <stack>
#include <twist/ed/stdlike/atomic.hpp>
#include <exe/executors/task.hpp>

namespace exe::executors::support {

class IntrusiveLockFreePushStack {
  using IntrusiveTask = exe::executors::IntrusiveTask;
  using IntrusiveTaskNode = wheels::IntrusiveListNode<IntrusiveTask>;

 public:
  IntrusiveLockFreePushStack() = default;
  IntrusiveLockFreePushStack(IntrusiveLockFreePushStack& other)
      : top_(other.top_.load()) {
  }

  void Push(IntrusiveTaskNode* obj) {
    auto current_top = top_.load();
    obj->prev_ = current_top;
    while (!top_.compare_exchange_strong(current_top, obj)) {
      obj->prev_ = current_top;
    }
  }

  IntrusiveLockFreePushStack GetReversedStack() {
    IntrusiveLockFreePushStack stack;

    auto node = top_.exchange(nullptr);
    while (node != nullptr) {
      auto next = node->prev_;
      stack.Push(node->AsItem());
      node = next;
    }

    return stack;
  }

  IntrusiveTask* Pop() {
    return top_.exchange(top_.load()->prev_)->AsItem();
  }

  bool IsEmpty() {
    return top_.load() == nullptr;
  }

 private:
  twist::ed::stdlike::atomic<IntrusiveTaskNode*> top_{nullptr};
};

}  // namespace exe::executors::support