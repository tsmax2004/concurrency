#include <stack>
#include <twist/ed/stdlike/atomic.hpp>
#include <exe/executors/task.hpp>

class IntrusiveLockFreePushStack {
  using IntrusiveTask = exe::executors::IntrusiveTask;
  using IntrusiveTaskNode = wheels::IntrusiveListNode<IntrusiveTask>;

 public:
  IntrusiveLockFreePushStack() = default;

  void Push(IntrusiveTaskNode* obj) {
    auto current_top = top_.load();
    obj->prev_ = current_top;
    while (!top_.compare_exchange_strong(current_top, obj)) {
      obj->prev_ = current_top;
    }
  }

  std::stack<IntrusiveTask*> GetReversedStack() {
    std::stack<IntrusiveTask*> stack;

    auto node = top_.exchange(nullptr);
    while (node != nullptr) {
      stack.push(node->AsItem());
      node = node->prev_;
    }

    return stack;
  }

 private:
  twist::ed::stdlike::atomic<IntrusiveTaskNode*> top_{nullptr};
};