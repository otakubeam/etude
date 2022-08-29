#pragma once

#include <vm/rt/primitive.hpp>

#include <cstdlib>
#include <vector>

namespace vm {

class VmStack {
 public:
  void Push(rt::PrimitiveValue value) {
    stack_.push_back(std::move(value));
  }

  auto Pop() -> rt::PrimitiveValue {
    auto r = std::move(stack_.back());
    stack_.pop_back();
    return r;
  }

 private:
  std::vector<rt::PrimitiveValue> stack_;
};

}  // namespace vm
