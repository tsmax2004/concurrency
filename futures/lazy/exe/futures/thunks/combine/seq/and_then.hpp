#pragma once

#include <exe/futures/thunks/combine/seq/base.hpp>

namespace exe::futures::thunks {

template <Thunk Producer, typename MapFun,
          typename InputValueType = typename Producer::ValueType,
          typename ValueTypeT = result::traits::ValueOf<
              std::invoke_result_t<MapFun, InputValueType>>>
struct [[nodiscard]] AndThen : SeqThunk<Producer, ValueTypeT> {
  using ValueType = ValueTypeT;

 public:
  AndThen(Producer p, MapFun f)
      : SeqThunk<Producer, ValueType>(std::move(p)),
        map_fun_(std::move(f)) {
  }

  Result<ValueType> PerformResult() override {
    return SeqThunk<Producer, ValueType>::input_->result.and_then(
        std::move(map_fun_));
  }

 private:
  MapFun map_fun_;
};

}  // namespace exe::futures::thunks
