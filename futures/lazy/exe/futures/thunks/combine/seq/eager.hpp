#pragma once

#include <exe/futures/model/thunk.hpp>

#include <exe/futures/containers/eager.hpp>

namespace exe::futures::thunks {

template <typename ValueTypeT>
struct [[nodiscard]] Eager {
  using ValueType = ValueTypeT;

 public:
  template <Thunk Wrapped>
  explicit Eager(Wrapped thunk) {
    container_ = new detail::EagerContainer(std::move(thunk));
  }

  explicit Eager(detail::Container<ValueType>* c)
      : container_(c) {
  }

  // Non-copyable
  Eager(const Eager&) = default;

  // Movable
  Eager(Eager&& other) = default;

  void Start(IConsumer<ValueType>* consumer) {
    container_->Start(consumer);
  }

 private:
  detail::Container<ValueType>* container_;
};

}  // namespace exe::futures::thunks
