#pragma once

#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <function2/function2.hpp>

#include <exception>

// Simple stackful coroutine

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
    NOT_STARTED,
    RUNNING,
    SUSPENDED,
    EXCEPTION,
    TERMINATED,
  };

  class CoroutineTrampoline : private sure::ITrampoline {
    friend Coroutine;

   private:
    explicit CoroutineTrampoline(Routine routine);

    void AllocateStack();

    void ReleaseStack();

    void SetupContext();

    void Run() noexcept override;

    Routine routine_;
    sure::Stack routine_stack_;
    sure::ExecutionContext context_;
  };

  void Dispatch();

  void SwitchToMain();

  static void RethrowException(std::exception_ptr);

  static void Terminate();

  sure::ExecutionContext main_context_;
  CoroutineTrampoline coroutine_trampoline_;

  CoroutineState state_;
  std::exception_ptr eptr_;
};
