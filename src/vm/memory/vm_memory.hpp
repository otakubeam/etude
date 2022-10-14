#pragma once

#include <vm/memory/mem_access.hpp>

#include <vm/elf_file.hpp>

#include <utility>

namespace vm::debug {
class Debugger;
}

namespace vm::memory {

class VmMemory {
  friend vm::debug::Debugger;

 public:
  VmMemory(size_t overall_size, size_t stack_size)
      : memory_{new uint8_t[overall_size]()}, stack_size_{stack_size} {
  }

  void Load(ElfFile elf) {
    program_text_ = std::move(elf);
  }

  auto GetStackArea() -> uint8_t* {
    return memory_ + stack_size_;
  }

  auto AccessMemory(MemAccess descriptor) -> uint8_t* {
    access_log_.push_back(descriptor);

    switch (descriptor.reference.tag) {
      case rt::ValueTag::InstrRef: {
        auto addr = descriptor.reference.as_ref.to_instr;
        auto chunk = program_text_->GetTextSection(addr.chunk_no);

        if (addr.instr_no < chunk.length) {
          return &chunk.text[addr.instr_no];
        }
        return nullptr;
      }

      case rt::ValueTag::HeapRef:
        return (uint8_t*)(descriptor.reference.as_ref.to_data +
                          (rt::PrimitiveValue*)(memory_));

      case rt::ValueTag::StackRef:
        return (uint8_t*)(descriptor.reference.as_ref.to_data +
                          (rt::PrimitiveValue*)GetStackArea());

      case rt::ValueTag::StaticRef:
        FMT_ASSERT(false, "Unimplemented!");

      default:
        FMT_ASSERT(false, "Unreachable!");
    }
  }

  auto FlushAccessLog() -> std::vector<MemAccess> {
    return std::exchange(access_log_, {});
  }

  auto AllocateMemory(uint32_t how_much) {
    uint32_t mark = allocated_;
    allocated_ += how_much;
    allocation_sizes_.emplace(mark, how_much);
    return rt::PrimitiveValue{
        .tag = rt::ValueTag::HeapRef,
        .as_ref = {.to_data = mark},
    };
  }

  auto LookupSize(uint32_t mark) {
    if (allocation_sizes_.contains(mark)) {
      return allocation_sizes_.find(mark);
    }

    auto it = allocation_sizes_.lower_bound(mark);
    return --it;
  }

  // I want to see it an undicriminated array of bytes
  // But I also want it to have some structure

 private:
  uint8_t* memory_;
  uint32_t allocated_ = 0;

  // What else do I want to know about an allocation?
  std::map<uint32_t, uint32_t> allocation_sizes_;

  size_t stack_size_ = 0;

  std::vector<MemAccess> access_log_;

  std::optional<ElfFile> program_text_;
};

}  // namespace vm::memory
