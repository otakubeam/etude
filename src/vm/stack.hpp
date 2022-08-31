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

  void PopCount(size_t count) {
    sp_ -= count;
  }

  void Ret() {
    // Move the stack pointer
    sp_ = fp_;

    // Expect by ABI to be an integer offset
    FMT_ASSERT(stack_.at(fp_).tag == rt::ValueTag::Int,
               "Expected an Int in fp slot");

    // Move the frame pointer
    fp_ = stack_.at(fp_).as_int;
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

  auto GetFnArg(size_t count) -> rt::PrimitiveValue {
    return GetAtFp(-3 - count);
  }

  auto GetLocalVar(size_t count) -> rt::PrimitiveValue {
    return GetAtFp(count);
  }

  auto GetSavedIp() -> int {
    return GetAtFp(-1).as_int;
  }

  void PrintStack() const {
    fmt::print("[!] Stack:\n");

    for (int i = 0; i < 16; i++) {
      fmt::print("{}\t", i);
    }

    fmt::print("\n");

    for (int i = 0; i < 16; i++) {
      fmt::print("{}\t", stack_.at(i).as_int);
    }

    fmt::print("\n");

    for (int i = 0; i < 16; i++) {
      if (i == (int)sp_) {
        fmt::print("sp\t");
      } else if (i == (int)fp_) {
        fmt::print("fp\t");
      } else {
        fmt::print("  \t");
      }
    }

    fmt::print("\n");
    fmt::print("\n");
  }

 private:
  auto Top() -> rt::PrimitiveValue {
    return stack_.at(sp_ - 1);
  }

  auto GetAtFp(int offset) -> rt::PrimitiveValue {
    return stack_.at(fp_ + offset);
  }

 private:
  // Stack pointer, frame pointer
  size_t sp_ = 0;
  size_t fp_ = 0;

  std::vector<rt::PrimitiveValue> stack_{65536};
};

}  // namespace vm
