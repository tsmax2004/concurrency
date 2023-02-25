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
    auto key = twist::ed::PrepareWake(sleep_);
    sleep_.fetch_add(1);
    twist::ed::WakeOne(key);
  }

  void NotifyAll() {
    auto key = twist::ed::PrepareWake(sleep_);
    sleep_.fetch_add(1);
    twist::ed::WakeAll(key);
  }

  ~CondVar() {
    NotifyAll();
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> sleep_{0};
};

}  // namespace stdlike
