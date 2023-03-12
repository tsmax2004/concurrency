#include "coroutine.hpp"

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/assert.hpp>
#include <wheels/core/compiler.hpp>

static const size_t kDefaultStackSize = 64 * 1024;

twist::ed::ThreadLocalPtr<Coroutine> current_coroutine;

Coroutine::Coroutine(Routine routine)
    : callee_(std::move(routine)) {
}

void Coroutine::Resume() {
  Coroutine* previous_coroutine = current_coroutine;
  current_coroutine = this;
  state_ = CoroutineState::Running;

  SwitchToCallee();

  current_coroutine = previous_coroutine;

  Dispatch();
}

void Coroutine::Suspend() {
  current_coroutine->state_ = CoroutineState::Suspended;
  current_coroutine->SwitchToCaller();
}

bool Coroutine::IsCompleted() const {
  return state_ == CoroutineState::Terminated;
}

void Coroutine::Terminate() {
  current_coroutine->state_ = CoroutineState::Terminated;
  current_coroutine->ExitToCaller();
}

void Coroutine::SetException(std::exception_ptr eptr) {
  current_coroutine->eptr_ = std::move(eptr);
}

void Coroutine::SwitchToCallee() {
  caller_context_.SwitchTo(callee_.context_);
}

void Coroutine::SwitchToCaller() {
  callee_.context_.SwitchTo(caller_context_);
}

void Coroutine::ExitToCaller() {
  callee_.context_.ExitTo(caller_context_);
}

void Coroutine::Dispatch() {
  switch (state_) {
    case CoroutineState::Suspended:
      break;
    case CoroutineState::Terminated:
      callee_.ReleaseStack();
      break;
    default:
      // Unexpected coroutine state
      break;
  }

  if (eptr_) {
    auto tmp_eptr = std::move(eptr_);
    eptr_ = nullptr;
    std::rethrow_exception(std::move(tmp_eptr));
  }
}

Coroutine::Callee::Callee(Routine routine)
    : routine_(std::move(routine)) {
  AllocateStack();
  SetupContext();
}

void Coroutine::Callee::AllocateStack() {
  stack_ = sure::Stack::AllocateBytes(kDefaultStackSize);
}

void Coroutine::Callee::ReleaseStack() {
  stack_.Release();
}

void Coroutine::Callee::SetupContext() {
  context_.Setup(stack_.MutView(), this);
}

void Coroutine::Callee::Run() noexcept {
  try {
    routine_();
  } catch (...) {
    Coroutine::SetException(std::move(std::current_exception()));
  }

  Coroutine::Terminate();

  WHEELS_UNREACHABLE();
}
