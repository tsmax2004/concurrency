#pragma once

namespace exe::threads::support {

inline void CpuRelax() {
#if defined(__TWIST_ARCH_x86_64)
  asm volatile("pause\n" : : : "memory");
#elif defined(__TWIST_ARCH_armv8_a_64)
  asm volatile("yield\n" : : : "memory");
#else
  ;
#endif
}

struct SpinWait {
 public:
  void operator()() {
    CpuRelax();
  }
};

}  // namespace exe::threads::support