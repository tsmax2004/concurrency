#pragma once

#include <exe/futures/types/future.hpp>
#include <exe/executors/executor.hpp>

#include <exe/result/traits/value_of.hpp>

#include <exe/futures/make/just.hpp>
#include <exe/futures/combine/seq/via.hpp>
#include <exe/futures/combine/seq/and_then.hpp>

#include <type_traits>

namespace exe::futures {

namespace traits {

template <typename F>
using SubmitT = result::traits::ValueOf<std::invoke_result_t<F>>;

}  // namespace traits

template <typename F>
Future<traits::SubmitT<F>> auto Submit(executors::IExecutor& exe, F fun) {
  auto tmp = [f = std::move(fun)](Unit) {
    return f();
  };
  return futures::Just() | futures::Via(exe) | futures::AndThen(std::move(tmp));
}

}  // namespace exe::futures
