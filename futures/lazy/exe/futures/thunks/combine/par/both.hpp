#pragma once

#include <twist/ed/stdlike/atomic.hpp>

#include <exe/futures/model/thunk.hpp>

#include <exe/futures/containers/both.hpp>

namespace exe::futures::thunks {

template <Thunk Producer1, Thunk Producer2>
struct [[nodiscard]] Both {
  using ValueType1 = typename Producer1::ValueType;
  using ValueType2 = typename Producer2::ValueType;
  using ValueType = std::tuple<ValueType1, ValueType2>;

 public:
  Both(Producer1 p1, Producer2 p2) {
    container_ = new detail::BothContainer(std::move(p1), std::move(p2));
  }

  // Non-copyable
  Both(const Both&) = delete;

  // Movable
  Both(Both&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    container_->Start(consumer);
  }

 private:
  detail::BothContainer<Producer1, Producer2>* container_;
};

}  // namespace exe::futures::thunks
