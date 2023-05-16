#pragma once

#include <optional>

#include <exe/threads/event.hpp>

#include <exe/futures/syntax/pipe.hpp>

#include <exe/futures/traits/value_of.hpp>

#include <exe/result/types/result.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Get {
  template <SomeFuture InputFuture>
  Result<traits::ValueOf<InputFuture>> Pipe(InputFuture future) {
    Consumer<traits::ValueOf<InputFuture>> consumer;
    return std::move(consumer.Get(std::move(future)));
  }

 private:
  template <typename T>
  struct Consumer : IConsumer<T> {
   public:
    template <SomeFuture Future>
    Result<T> Get(Future future) {
      future.Start(this);

      is_ready_.Wait();
      return std::move(*result_);
    }

   private:
    void Consume(Output<T> output) noexcept override {
      result_.emplace(std::move(output.result));
      is_ready_.Set();
    }

    threads::Event is_ready_;
    std::optional<Result<T>> result_;
  };
};

}  // namespace pipe

inline auto Get() {
  return pipe::Get{};
}

}  // namespace exe::futures
