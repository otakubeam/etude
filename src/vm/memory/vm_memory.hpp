#pragma once

#include <vm/memory/mem_access.hpp>

#include <vm/elf_file.hpp>

namespace vm::memory {

class VmMemory {
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
      case rt::ValueTag::StaticRef:
        FMT_ASSERT(false, "Unimplemented!");

      case rt::ValueTag::StackRef:
        return (uint8_t*)(descriptor.reference.as_ref.to_data +
                          (rt::PrimitiveValue*)GetStackArea());
      default:
        FMT_ASSERT(false, "Unreachable!");
    }
  }

  // I want to see it an undicriminated array of bytes
  // But I also want it to have some structure

 private:
  uint8_t* memory_;

  size_t stack_size_ = 0;

  std::vector<MemAccess> access_log_;

  std::optional<ElfFile> program_text_;
};

}  // namespace vm::memory
