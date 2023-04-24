#include <exe/coro/core.hpp>

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/assert.hpp>
#include <wheels/core/compiler.hpp>

namespace exe::coro {

twist::ed::ThreadLocalPtr<CoroutineCore> current_coroutine;

CoroutineCore::CoroutineCore(Routine routine, wheels::MutableMemView stack)
    : routine_(std::move(routine)),
      state_(CoroutineState::Starting) {
  SetupContext(std::move(stack));
}

void CoroutineCore::Run() noexcept {
  try {
    routine_();
  } catch (...) {
    CoroutineCore::SetException(std::move(std::current_exception()));
  }

  CoroutineCore::Terminate();

  WHEELS_UNREACHABLE();
}

void CoroutineCore::Resume() {
  CoroutineCore* previous_coroutine = current_coroutine;
  current_coroutine = this;
  state_ = CoroutineState::Running;

  SwitchToCallee();

  current_coroutine = previous_coroutine;

  Dispatch();
}

void CoroutineCore::Suspend() {
  current_coroutine->state_ = CoroutineState::Suspended;
  current_coroutine->SwitchToCaller();
}

bool CoroutineCore::IsCompleted() const {
  return state_ == CoroutineState::Terminated;
}

void CoroutineCore::SetupContext(wheels::MutableMemView stack) {
  context_.Setup(std::move(stack), this);
}

void CoroutineCore::Dispatch() {
  switch (state_) {
    case CoroutineState::Suspended:
      break;
    case CoroutineState::Terminated:
      break;
    default:
      WHEELS_PANIC("Unexpected fiber state");
      break;
  }

  if (eptr_) {
    auto tmp_eptr = std::move(eptr_);
    eptr_ = nullptr;
    std::rethrow_exception(std::move(tmp_eptr));
  }
}

void CoroutineCore::Terminate() {
  current_coroutine->state_ = CoroutineState::Terminated;
  current_coroutine->ExitToCaller();
}

void CoroutineCore::SetException(std::exception_ptr eptr) {
  current_coroutine->eptr_ = std::move(eptr);
}

void CoroutineCore::SwitchToCallee() {
  caller_context_.SwitchTo(context_);
}

void CoroutineCore::SwitchToCaller() {
  context_.SwitchTo(caller_context_);
}

void CoroutineCore::ExitToCaller() {
  context_.ExitTo(caller_context_);
}

}  // namespace exe::coro
