#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/thunks/combine/seq/apply.hpp>

#include <type_traits>

namespace exe::futures {

namespace pipe {

template <Thunk ProducerT, typename MapFunT>
struct ApplyTraitsMap {
  using Producer = ProducerT;
  using MapFun = MapFunT;
  using InputValueType = typename Producer::ValueType;
  using ValueType = std::invoke_result_t<MapFun, InputValueType>;

  static bool Shortcut(Result<InputValueType>& input) {
    return !input.has_value();
  }

  static Result<ValueType> ComputeResult(Result<InputValueType> input,
                                         MapFun fun) {
    return input.map(std::move(fun));
  }
};

template <typename F>
struct [[nodiscard]] Map {
  F fun;

  explicit Map(F f)
      : fun(std::move(f)) {
  }

  template <typename T>
  using U = std::invoke_result_t<F, T>;

  template <SomeFuture InputFuture>
  Future<U<traits::ValueOf<InputFuture>>> auto Pipe(InputFuture future) {
    return thunks::Apply<ApplyTraitsMap<InputFuture, F>>(std::move(future),
                                                         std::move(fun));
  }
};

}  // namespace pipe

// Future<T> -> (T -> U) -> Future<U>

template <typename F>
auto Map(F fun) {
  return pipe::Map{std::move(fun)};
}

}  // namespace exe::futures
