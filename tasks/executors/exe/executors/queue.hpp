#include <stack>
#include <twist/ed/stdlike/atomic.hpp>

template <typename T>
class LockFreePushStack {
 public:
  LockFreePushStack() = default;

  void Push(T obj) {
    auto new_top = new Node;
    new (new_top->obj) T(std::move(obj));

    auto current_top = top_.load();
    new_top->prev = current_top;
    while (!top_.compare_exchange_strong(current_top, new_top)) {
      new_top->prev = current_top;
    }
  }

  std::stack<T> GetReversedStack() {
    std::stack<T> stack;

    auto node = top_.exchange(nullptr);
    while (node != nullptr) {
      stack.push(std::move(*((T*)node->obj)));

      auto prev = node->prev;
      delete node;
      node = prev;
    }
    return std::move(stack);
  }

 private:
  struct Node {
    alignas(T) char obj[sizeof(T)];
    Node* prev;
  };

  twist::ed::stdlike::atomic<Node*> top_{nullptr};
};