#pragma once

#include <optional>

#include <exe/futures/model/thunk.hpp>

#include <exe/futures/containers/boxed.hpp>

namespace exe::futures::thunks {

template <typename T>
struct [[nodiscard]] Boxed {
  using ValueType = T;

 public:
  // Auto-boxing
  template <Thunk Wrapped>
  Boxed(Wrapped thunk) {  // NOLINT
    container_ = new detail::BoxedContainer(std::move(thunk));
  }

  // Non-copyable
  Boxed(const Boxed&) = delete;

  // Movable
  Boxed(Boxed&&) = default;

  void Start(IConsumer<T>* consumer) {
    container_->Start(consumer);
  }

 private:
  detail::Container<ValueType>* container_;
};

}  // namespace exe::futures::thunks
