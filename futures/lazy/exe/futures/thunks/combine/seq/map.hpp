#pragma once

#include <exe/futures/thunks/combine/seq/base.hpp>

namespace exe::futures::thunks {

template <Thunk Producer, typename MapFun,
          typename InputValueType = typename Producer::ValueType,
          typename ValueTypeT = std::invoke_result_t<MapFun, InputValueType>>
struct [[nodiscard]] Map final : SeqThunk<Producer, ValueTypeT> {
  using ValueType = ValueTypeT;

 public:
  Map(Producer p, MapFun f)
      : SeqThunk<Producer, ValueType>(std::move(p)),
        map_fun_(std::move(f)) {
  }

  // Non-copyable
  Map(const Map&) = delete;

  // Movable
  Map(Map&&) = default;

 private:
  Result<ValueType> PerformResult() override {
    return SeqThunk<Producer, ValueType>::input_->result.map(
        std::move(map_fun_));
  }

  MapFun map_fun_;
};

}  // namespace exe::futures::thunks
