#pragma once

#include <sure/stack.hpp>

#include <optional>
#include <vector>

// Source:
// https://gitlab.com/Lipovsky/tinyfibers/-/blob/master/tf/rt/stack_allocator.hpp

namespace exe::coro {

static const size_t kDefaultStackSize = 64 * 1024;

class StackAllocator {
  using Stack = sure::Stack;

 public:
  Stack Allocate() {
    if (auto stack = TryTakeFromPool()) {
      return std::move(*stack);
    }
    return AllocateNew();
  }

  void Release(Stack stack) {
    pool_.push_back(std::move(stack));
  }

 private:
  static Stack AllocateNew() {
    return Stack::AllocateBytes(/*at_least=*/kDefaultStackSize);
  }
  std::optional<Stack> TryTakeFromPool() {
    if (pool_.empty()) {
      return std::nullopt;
    }

    Stack stack = std::move(pool_.back());
    pool_.pop_back();
    return stack;
  }

 private:
  std::vector<Stack> pool_;
};

}  // namespace exe::coro
