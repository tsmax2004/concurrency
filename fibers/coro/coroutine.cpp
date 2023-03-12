#include "coroutine.hpp"

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/assert.hpp>
#include <wheels/core/compiler.hpp>

static const size_t kDefaultStackSize = 64 * 1024;

twist::ed::ThreadLocalPtr<Coroutine> current_coroutine;

Coroutine::Coroutine(Routine routine)
    : runnable_(std::move(routine)) {
  CreateCore();
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

void Coroutine::CreateCore() {
  stack_ = AllocateStack();
  core_ = std::make_shared<CoroutineCore>(stack_.MutView(), &runnable_);
}

void Coroutine::DestroyCore() {
  ReleaseStack(std::move(stack_));
  core_.reset();
}

sure::Stack Coroutine::AllocateStack() {
  return sure::Stack::AllocateBytes(kDefaultStackSize);
}

void Coroutine::ReleaseStack(sure::Stack stack) {
  stack.Release();
}

void Coroutine::SwitchToCallee() {
  caller_context_.SwitchTo(core_->GetContext());
}

void Coroutine::SwitchToCaller() {
  core_->GetContext().SwitchTo(caller_context_);
}

void Coroutine::ExitToCaller() {
  core_->GetContext().ExitTo(caller_context_);
}

void Coroutine::Dispatch() {
  switch (state_) {
    case CoroutineState::Suspended:
      break;
    case CoroutineState::Terminated:
      DestroyCore();
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

CoroutineCore::CoroutineCore(wheels::MutableMemView stack_view,
                             sure::ITrampoline* runnable)
    : runnable_(runnable) {
  SetupContext(std::move(stack_view));
}

sure::ExecutionContext& CoroutineCore::GetContext() {
  return context_;
}

void CoroutineCore::SetupContext(wheels::MutableMemView stack_view) {
  context_.Setup(std::move(stack_view), runnable_);
}

Coroutine::IRunnable::IRunnable(Routine routine)
    : routine_(std::move(routine)) {
}

void Coroutine::IRunnable::Run() noexcept {
  try {
    routine_();
  } catch (...) {
    Coroutine::SetException(std::move(std::current_exception()));
  }

  Coroutine::Terminate();

  WHEELS_UNREACHABLE();
}