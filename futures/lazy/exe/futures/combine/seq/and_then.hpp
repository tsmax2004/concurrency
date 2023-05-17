#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>
#include <exe/result/traits/value_of.hpp>

#include <exe/futures/thunks/combine/seq/apply.hpp>

#include <type_traits>

namespace exe::futures {

namespace pipe {

template <Thunk ProducerT, typename MapFunT>
struct ApplyTraitsAndThen {
  using Producer = ProducerT;
  using MapFun = MapFunT;
  using InputValueType = typename Producer::ValueType;
  using ValueType = result::traits::ValueOf<
      std::invoke_result_t<MapFun, typename Producer::ValueType>>;

  static bool Shortcut(Result<InputValueType>& input) {
    return !input.has_value();
  }

  static Result<ValueType> ComputeResult(Result<InputValueType> input,
                                         MapFun fun) {
    return input.and_then(std::move(fun));
  }
};

template <typename F>
struct [[nodiscard]] AndThen {
  F fun;

  explicit AndThen(F f)
      : fun(std::move(f)) {
  }

  // Non-copyable
  AndThen(AndThen&) = delete;

  template <typename T>
  using U = result::traits::ValueOf<std::invoke_result_t<F, T>>;

  template <SomeFuture InputFuture>
  Future<U<traits::ValueOf<InputFuture>>> auto Pipe(InputFuture future) {
    return thunks::Apply<ApplyTraitsAndThen<InputFuture, F>>{std::move(future),
                                                             std::move(fun)};
  }
};

}  // namespace pipe

// Future<T> -> (T -> Result<U>) -> Future<U>

template <typename F>
auto AndThen(F fun) {
  return pipe::AndThen{std::move(fun)};
}

}  // namespace exe::futures
