#pragma once

#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <function2/function2.hpp>

#include <exception>

namespace exe::coro {

class Coroutine {
 public:
  using Routine = fu2::unique_function<void()>;

  explicit Coroutine(Routine routine);

  void Resume();

  // Suspend running coroutine
  static void Suspend();

  bool IsCompleted() const;

 private:
  enum class CoroutineState {
    Starting,
    Running,
    Suspended,
    Terminated,
  };

  class Callee : sure::ITrampoline {
    friend Coroutine;

   private:
    explicit Callee(Routine routine);

    void AllocateStack();
    void ReleaseStack();
    void SetupContext();

    [[noreturn]] void Run() noexcept override;

    Routine routine_;
    sure::Stack stack_;
    sure::ExecutionContext context_;
  };

  static void Terminate();
  static void SetException(std::exception_ptr);

  void SwitchToCallee();
  void SwitchToCaller();
  void ExitToCaller();

  void Dispatch();

  Callee callee_;
  sure::ExecutionContext caller_context_;

  std::exception_ptr eptr_;

  CoroutineState state_;
};

}  // namespace exe::coro
