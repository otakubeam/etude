#pragma once

#include <vm/chunk.hpp>

#include <ast/scope/environment.hpp>
#include <ast/statements.hpp>

namespace vm::codegen::detail {

class StructSymbol {
  using Env = Environment<StructSymbol*>;

 public:
  StructSymbol(StructDeclStatement* node, Env& others)
      : node_{node}, others_{others} {
  }

  size_t Size() {
    size_t result = 0;

    for (size_t i = 0; i < node_->field_names_.size(); i++) {
      if (!node_->field_types_[i]->IsStruct()) {
        result += 1;  // primitive and fn_ptr types
        continue;
      }

      auto& name = node_->field_names_[i];
      result += others_.Get(name.GetName()).value()->Size();
    }

    return result;
  }

 private:
  StructDeclStatement* node_;

  Env& others_;

  // bool incomplete_ = true;
};

}  // namespace vm::codegen::detail
