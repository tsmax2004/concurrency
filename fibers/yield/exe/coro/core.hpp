#pragma once

#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <function2/function2.hpp>

#include <exception>
#include <memory>

namespace exe::coro {

class CoroutineCore {
 public:
  CoroutineCore(wheels::MutableMemView /* stack view*/,
                sure::ITrampoline* /* runnable */);

  sure::ExecutionContext& GetContext();

 private:
  void SetupContext(wheels::MutableMemView);

  sure::ExecutionContext context_;
  sure::ITrampoline* runnable_;
};

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

  class IRunnable : public sure::ITrampoline {
    friend Coroutine;

   private:
    explicit IRunnable(Routine);
    [[noreturn]] void Run() noexcept override;

    Routine routine_;
  };

  static void Terminate();
  static void SetException(std::exception_ptr);

  void CreateCore();
  void DestroyCore();

  sure::Stack AllocateStack();
  void ReleaseStack(sure::Stack);

  void SwitchToCallee();
  void SwitchToCaller();
  void ExitToCaller();

  void Dispatch();

  sure::Stack stack_;
  IRunnable runnable_;
  std::shared_ptr<CoroutineCore> core_;

  sure::ExecutionContext caller_context_;

  std::exception_ptr eptr_;
  CoroutineState state_;
};

}  // namespace exe::coro
