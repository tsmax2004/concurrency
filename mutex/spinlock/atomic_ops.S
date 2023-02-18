#if(APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicLoad)
.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)

FUNCTION_NAME(AtomicLoad):
    lock movq (%rdi), %rax
    retq

FUNCTION_NAME(AtomicStore):
    lock movq  %rsi, (%rdi)
    retq

FUNCTION_NAME(AtomicExchange):
    lock movq    (%rdi), %rax  # Load
loop:
    lock cmpxchg %rsi, (%rdi)  # TryStore
    jne     loop
    retq
