#include "coroutine.hpp"

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/assert.hpp>
#include <wheels/core/compiler.hpp>

#include <wheels/core/size_literals.hpp>

// using namespace wheels::size_literals;
static const size_t kDefaultStackSize = 64 * 1024;

twist::ed::ThreadLocalPtr<Coroutine> current_coroutine;

Coroutine::Coroutine(Routine routine)
    : coroutine_trampoline_(std::move(routine)) {
}

void Coroutine::Resume() {
  prev_coroutine_ = current_coroutine;
  current_coroutine = this;

  state_ = CoroutineState::RUNNING;

  main_context_.SwitchTo(coroutine_trampoline_.context_);
  current_coroutine = prev_coroutine_;

  Dispatch();
}

void Coroutine::Suspend() {
  current_coroutine->state_ = CoroutineState::SUSPENDED;
  current_coroutine->SwitchToMain();
}

bool Coroutine::IsCompleted() const {
  return state_ == CoroutineState::TERMINATED;
}

void Coroutine::Dispatch() {
  switch (state_) {
    case CoroutineState::EXCEPTION:
      state_ = CoroutineState::TERMINATED;
      coroutine_trampoline_.ReleaseStack();
      std::rethrow_exception(std::move(eptr_));

    case CoroutineState::TERMINATED:
      coroutine_trampoline_.ReleaseStack();
      break;

    case CoroutineState::SUSPENDED:
      break;

    default:
      WHEELS_PANIC("Unexpected coroutine state");
      break;
  }
}

void Coroutine::SwitchToMain() {
  coroutine_trampoline_.context_.SwitchTo(main_context_);
}

void Coroutine::ExitToMain() {
  coroutine_trampoline_.context_.ExitTo(main_context_);
}

void Coroutine::RethrowException(std::exception_ptr eptr) {
  current_coroutine->state_ = CoroutineState::EXCEPTION;
  current_coroutine->eptr_ = std::move(eptr);
  current_coroutine->ExitToMain();
}

void Coroutine::Terminate() {
  current_coroutine->state_ = CoroutineState::TERMINATED;
  current_coroutine->ExitToMain();
}

Coroutine::CoroutineTrampoline::CoroutineTrampoline(Routine routine)
    : routine_(std::move(routine)) {
  AllocateStack();
  SetupContext();
}

void Coroutine::CoroutineTrampoline::AllocateStack() {
  routine_stack_ = sure::Stack::AllocateBytes(kDefaultStackSize);
}

void Coroutine::CoroutineTrampoline::ReleaseStack() {
  routine_stack_.Release();
}

void Coroutine::CoroutineTrampoline::SetupContext() {
  context_.Setup(routine_stack_.MutView(), this);
}

void Coroutine::CoroutineTrampoline::Run() noexcept {
  try {
    routine_();
  } catch (...) {
    Coroutine::RethrowException(std::move(std::current_exception()));
  }

  Coroutine::Terminate();
}
