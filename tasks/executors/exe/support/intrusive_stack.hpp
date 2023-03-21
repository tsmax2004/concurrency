#include <stack>
#include <twist/ed/stdlike/atomic.hpp>
#include <exe/executors/task.hpp>

// template <typename TaskBaseNode>
// class LockFreePushStack {
//  public:
//   LockFreePushStack() = default;
//
//   void Push(TaskBaseNode obj) {
//     auto new_top = new Node;
//     new (new_top->obj) TaskBaseNode(std::move(obj));
//
//     auto current_top = top_.load();
//     new_top->prev = current_top;
//     while (!top_.compare_exchange_strong(current_top, new_top)) {
//       new_top->prev = current_top;
//     }
//   }
//
//   std::stack<TaskBaseNode> GetReversedStack() {
//     std::stack<TaskBaseNode> stack;
//
//     auto node = top_.exchange(nullptr);
//     while (node != nullptr) {
//       stack.push(std::move(*((TaskBaseNode*)node->obj)));
//
//       auto prev = node->prev;
//       delete node;
//       node = prev;
//     }
//     return std::move(stack);
//   }
//
//  private:
//   struct Node {
//     alignas(TaskBaseNode) char obj[sizeof(TaskBaseNode)];
//     Node* prev;
//   };
//
//   twist::ed::stdlike::atomic<Node*> top_{nullptr};
// };

class IntrusiveLockFreePushStack {
  using TaskBaseNode = wheels::IntrusiveListNode<exe::executors::TaskBase>;

 public:
  IntrusiveLockFreePushStack() = default;

  void Push(TaskBaseNode* obj) {
    auto current_top = top_.load();
    obj->prev_ = current_top;
    while (!top_.compare_exchange_strong(current_top, obj)) {
      obj->prev_ = current_top;
    }
  }

  std::stack<TaskBaseNode*> GetReversedStack() {
    std::stack<TaskBaseNode*> stack;

    auto node = top_.exchange(nullptr);
    while (node != nullptr) {
      stack.push(node);
      node = node->prev_;
    }

    return stack;
  }

 private:
  twist::ed::stdlike::atomic<TaskBaseNode*> top_{nullptr};
};