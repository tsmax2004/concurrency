#pragma once

#include <exe/futures/thunks/combine/seq/base.hpp>

namespace exe::futures::thunks {

template <Thunk Producer, typename MapFun,
          typename InputValueType = typename Producer::ValueType,
          typename ValueTypeT = InputValueType>
struct [[nodiscard]] OrElse final : SeqThunk<Producer, ValueTypeT> {
  using ValueType = ValueTypeT;

 public:
  OrElse(Producer p, MapFun f)
      : SeqThunk<Producer, ValueType>(std::move(p)),
        map_fun_(std::move(f)) {
  }

  // Non-copyable
  OrElse(const OrElse&) = delete;

  // Movable
  OrElse(OrElse&&) = default;

 private:
  Result<ValueType> PerformResult() override {
    return SeqThunk<Producer, ValueType>::input_->result.or_else(
        std::move(map_fun_));
  }

  MapFun map_fun_;
};

}  // namespace exe::futures::thunks
