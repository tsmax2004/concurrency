#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdint>

namespace stdlike {

class CondVar {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& mutex) {
    auto old = sleep_.fetch_add(1);
    mutex.unlock();
    twist::ed::Wait(sleep_, old + 1);
    mutex.lock();
  }

  void NotifyOne() {
    sleep_.fetch_add(1);
    twist::ed::WakeOne(twist::ed::PrepareWake(sleep_));
  }

  void NotifyAll() {
    sleep_.fetch_add(1);
    twist::ed::WakeAll(twist::ed::PrepareWake(sleep_));
  }

  ~CondVar() {
    NotifyAll();
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> sleep_{0};
};

}  // namespace stdlike
