#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/futures/model/thunk.hpp>

#include <exe/futures/containers/first.hpp>

namespace exe::futures::thunks {

template <Thunk Producer1, Thunk Producer2,
          typename ValueTypeT = typename Producer1::ValueType>
struct [[nodiscard]] First {
  using ValueType = ValueTypeT;

 public:
  First(Producer1 p1, Producer2 p2) {
    container_ = new detail::FirstContainer(std::move(p1), std::move(p2));
  }

  // Non-copyable
  First(const First&) = delete;

  // Movable
  First(First&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    container_->Start(consumer);
  }

 private:
  detail::FirstContainer<Producer1, Producer2>* container_;
};

}  // namespace exe::futures::thunks
