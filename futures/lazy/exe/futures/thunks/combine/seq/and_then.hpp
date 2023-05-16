#pragma once

#include <exe/futures/thunks/combine/seq/base.hpp>

#include <exe/result/traits/value_of.hpp>

namespace exe::futures::thunks {

namespace traits {

template <Thunk Producer, typename MapFun>
using ValueTypeT = result::traits::ValueOf<
    std::invoke_result_t<MapFun, typename Producer::ValueType>>;

}  // namespace traits

template <Thunk Producer, typename MapFun>
struct [[nodiscard]] AndThen final
    : SeqThunk<Producer, traits::ValueTypeT<Producer, MapFun>> {
  using ValueType = traits::ValueTypeT<Producer, MapFun>;

 public:
  AndThen(Producer p, MapFun f)
      : SeqThunk<Producer, ValueType>(std::move(p)),
        map_fun_(std::move(f)) {
  }

  // Non-copyable
  AndThen(const AndThen&) = delete;

  // Movable
  AndThen(AndThen&&) = default;

 private:
  Result<ValueType> PerformResult() override {
    return SeqThunk<Producer, ValueType>::input_->result.and_then(
        std::move(map_fun_));
  }

  MapFun map_fun_;
};

}  // namespace exe::futures::thunks
