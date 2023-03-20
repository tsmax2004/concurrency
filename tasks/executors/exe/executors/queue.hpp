#include <stack>
#include <twist/ed/stdlike/atomic.hpp>

template <typename T>
class StrandLockFreeQueue {
 public:
  StrandLockFreeQueue() = default;

  void Push(T obj) {
    auto new_top = new Node;
    new (new_top->obj) T(std::move(obj));

    auto current_top = push_top_.load();
    new_top->prev = current_top;
    while (!push_top_.compare_exchange_strong(current_top, new_top)) {
      new_top->prev = current_top;
    }
  }

  void PreparePop() {
    auto tmp_top = push_top_.exchange(nullptr);
    if (tmp_top == nullptr || tmp_top->prev == nullptr) {
      pop_top_.store(tmp_top);
      return;
    }

    auto prev = tmp_top;
    auto node = tmp_top->prev;
    tmp_top->prev = nullptr;
    while (node != nullptr) {
      auto next = node->prev;
      node->prev = prev;
      prev = node;
      node = next;
    }

    pop_top_.store(prev);
  }

  bool PopEmpty() {
    return pop_top_.load() == nullptr;
  }

  bool PushEmpty() {
    return push_top_ == nullptr;
  }

  T Pop() {
    auto node = pop_top_.load();

    pop_top_.store(node->prev);
    T obj = std::move(*((T*)node->obj));
    delete node;

    return std::move(obj);
  }

 private:
  struct Node {
    alignas(T) char obj[sizeof(T)];
    Node* prev;
  };

  twist::ed::stdlike::atomic<Node*> push_top_{nullptr};
  twist::ed::stdlike::atomic<Node*> pop_top_{nullptr};
};