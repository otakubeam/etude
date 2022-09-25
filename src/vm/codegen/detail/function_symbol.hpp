#pragma once

#include <ast/scope/environment.hpp>
#include <ast/statements.hpp>

namespace vm::codegen::detail {

class FunctionSymbol {
  using Env = Environment<FunctionSymbol*>;

 public:
  FunctionSymbol(size_t chunk_number) : chunk_number_{chunk_number} {
  }

  size_t GetAddr() {
    return chunk_number_;
  }

 private:
  // TODO: will be an address in VM memory
  size_t chunk_number_ = 0;
};

}  // namespace vm::codegen::detail
