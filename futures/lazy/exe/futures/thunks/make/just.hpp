#pragma once

#include <exe/futures/model/thunk.hpp>
#include <exe/result/make/ok.hpp>

namespace exe::futures::thunks {

struct [[nodiscard]] Just {
  using ValueType = Unit;

 public:
  explicit Just() {
  }

  // Non-copyable
  Just(const Just&) = delete;

  // Movable
  Just(Just&&) = default;

  void Start(IConsumer<ValueType>* consumer) {
    consumer->Complete(result::Ok(Unit()));
  }

 private:
};

}  // namespace exe::futures::thunks
