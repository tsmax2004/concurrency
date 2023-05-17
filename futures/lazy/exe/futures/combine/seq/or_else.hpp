#pragma once

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/futures/thunks/combine/seq/apply.hpp>

namespace exe::futures {

namespace pipe {

template <Thunk ProducerT, typename MapFunT>
struct ApplyTraitsOrElse {
  using Producer = ProducerT;
  using MapFun = MapFunT;
  using InputValueType = typename Producer::ValueType;
  using ValueType = InputValueType;

  static bool Shortcut(Result<InputValueType>& input) {
    return input.has_value();
  }

  static Result<ValueType> ComputeResult(Result<InputValueType> input,
                                         MapFun fun) {
    return input.or_else(std::move(fun));
  }
};

template <typename F>
struct [[nodiscard]] OrElse {
  F fun;

  explicit OrElse(F f)
      : fun(std::move(f)) {
  }

  template <SomeFuture InputFuture>
  Future<traits::ValueOf<InputFuture>> auto Pipe(InputFuture future) {
    return thunks::Apply<ApplyTraitsOrElse<InputFuture, F>>(std::move(future),
                                                            std::move(fun));
  }
};

}  // namespace pipe

// Future<T> -> (Error -> Result<T>) -> Future<T>

template <typename F>
auto OrElse(F fun) {
  return pipe::OrElse{std::move(fun)};
}

}  // namespace exe::futures
