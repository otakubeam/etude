#pragma once

#include <vm/memory/vm_memory.hpp>

#include <vm/rt/primitive.hpp>

#include <cstdlib>
#include <vector>

// Forward declare class from another namespace
namespace vm::debug {
class StackPrinter;
}

namespace vm::memory {

class VmStack {
 public:
  friend class debug::StackPrinter;

  VmStack(VmMemory& memory)
      : memory_{memory},
        stack_area_{(rt::PrimitiveValue*)memory.GetStackArea()} {
    // Set the first fp to do nothing
    stack_area_[0] = {
        .tag = rt::ValueTag::StackRef,
        .as_ref = {.to_data = 0},
    };
  }

  void Push(rt::PrimitiveValue value) {
    memory_.AccessMemory(MakeMemoryAccess(sp_, true));
    stack_area_[sp_] = std::move(value);
    sp_ += 1;
  }

  auto Pop() -> rt::PrimitiveValue {
    sp_ -= 1;
    memory_.AccessMemory(MakeMemoryAccess(sp_, false));
    return stack_area_[sp_];
  }

  void PopCount(size_t count) {
    sp_ -= count;
  }

  bool Ret() {
    // Move the stack pointer
    sp_ = fp_;

    // Expect by ABI to be a StackRef
    FMT_ASSERT(stack_area_[fp_].tag == rt::ValueTag::StackRef,
               "Expected an StackRef in fp slot");

    // Move the frame pointer
    fp_ = stack_area_[fp_].as_ref.to_data;

    return sp_ != 0;
  }

  void TailRet() {
    // Move the stack pointer
    sp_ = fp_ + 1;
  }

  void PrepareCallframe() {
    // Save curent fp in sp
    stack_area_[sp_] = rt::PrimitiveValue{
        .tag = rt::ValueTag::StackRef,
        .as_ref = rt::Reference{.to_data = (uint32_t)fp_},
    };

    // Move fp to sp
    fp_ = sp_;

    // Point to the next empty slot
    sp_ += 1;
  }

  void StoreAtFp(int16_t offset, const rt::PrimitiveValue& value) {
    memory_.AccessMemory(MakeMemoryAccess(fp_ + offset, true));
    stack_area_[fp_ + offset] = value;
  }

  auto GetAtFp(int offset) {
    memory_.AccessMemory(MakeMemoryAccess(fp_ + offset, false));
    return Push(stack_area_[fp_ + offset]);
  }

  uint32_t GetFp() const {
    return fp_;
  }

  auto Top() -> rt::PrimitiveValue& {
    memory_.AccessMemory(MakeMemoryAccess(sp_ - 1, true));
    return stack_area_[sp_ - 1];
  }

 private:
  MemAccess MakeMemoryAccess(uint32_t data, bool is_store) {
    return MemAccess{
        .reference =
            {
                .tag = rt::ValueTag::StackRef,
                .as_ref = {.to_data = data},
            },
        .store = is_store,
    };
  }

 private:
  // Stack pointer, frame pointer
  size_t sp_ = 1;
  size_t fp_ = 0;

  VmMemory& memory_;

  rt::PrimitiveValue* stack_area_ = nullptr;
};

}  // namespace vm::memory
