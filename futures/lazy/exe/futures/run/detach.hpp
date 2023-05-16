#pragma once

#include <exe/futures/syntax/pipe.hpp>

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Detach {
  template <SomeFuture Future>
  void Pipe(Future f) {
    auto* runner = new Runner(std::move(f));
    runner->Start();
  }

 private:
  template <SomeFuture Future>
  struct Runner : IConsumer<typename Future::ValueType> {
    using ValueType = typename Future::ValueType;

   public:
    explicit Runner(Future f)
        : future_(std::move(f)) {
    }

    void Start() {
      future_.Start(this);
    }

   private:
    void Consume(Output<ValueType>) noexcept override {
      delete this;
    }

    Future future_;
  };
};

}  // namespace pipe

inline auto Detach() {
  return pipe::Detach{};
}

}  // namespace exe::futures
