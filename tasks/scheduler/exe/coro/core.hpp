#pragma once

#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <exe/coro/routine.hpp>

#include <exception>

namespace exe::coro {

class CoroutineCore : public ::sure::ITrampoline {
 public:
  CoroutineCore(Routine routine, wheels::MutableMemView stack);

  void Resume();
  void Suspend();

  bool IsCompleted() const;

 private:
  void Run() noexcept override;

 private:
  enum class CoroutineState {
    Starting,
    Running,
    Suspended,
    Terminated,
  };

  void SetupContext(wheels::MutableMemView);

  static void Terminate();
  static void SetException(std::exception_ptr);

  void SwitchToCallee();
  void SwitchToCaller();
  void ExitToCaller();

  void Dispatch();

  Routine routine_;
  sure::ExecutionContext context_;

  sure::ExecutionContext caller_context_;

  std::exception_ptr eptr_;
  CoroutineState state_;
};

}  // namespace exe::coro
