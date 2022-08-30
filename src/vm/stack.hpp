#pragma once

#include <vm/rt/primitive.hpp>

#include <cstdlib>
#include <vector>

namespace vm {

class VmStack {
 public:
  void Push(rt::PrimitiveValue value) {
    stack_.at(sp_) = std::move(value);
    sp_ += 1;
  }

  auto Pop() -> rt::PrimitiveValue {
    sp_ -= 1;
    return stack_.at(sp_);
  }

  void Ret() {
    // Move the stack pointer
    sp_ = fp_;

    // Move the frame pointer
    fp_ = stack_.at(fp_).as_int;

    // Expect by ABI to be an integer offset
    FMT_ASSERT(stack_[fp_].tag == rt::ValueTag::Int,
               "Expected an Int in fp slot");
  }

  void PrepareCallframe() {
    // Save curent fp in sp
    stack_.at(sp_) = rt::PrimitiveValue{.tag = rt::ValueTag::Int,  //
                                        .as_int = (int)fp_};

    // Move fp to sp
    fp_ = sp_;

    // Point to the next empty slot
    sp_ += 1;
  }

 private:
  auto Top() -> rt::PrimitiveValue {
    return stack_.at(sp_ - 1);
  }

 private:
  // Stack pointer, frame pointer
  size_t sp_ = 0;
  size_t fp_ = 0;

  std::vector<rt::PrimitiveValue> stack_{65536};
};

}  // namespace vm
