#pragma once

#include <vm/debug/stack_printer.hpp>
#include <vm/memory/mem_access.hpp>
#include <vm/elf_file.hpp>
#include <vm/stack_v2.hpp>

#include <fmt/color.h>

namespace vm::memory {

class VmMemory {
 public:
  VmMemory(size_t overall_size, size_t stack_size)
      : memory_{new uint8_t[overall_size]()}, stack_(memory_ + stack_size) {
  }

  void Load(ElfFile elf) {
    program_text_ = std::move(elf);
  }

  auto GetStack() -> VmStack& {
    stack_printer_.Print();
    return stack_;
  }

  auto AccessMemory(MemAccess descriptor) -> uint8_t* {
    switch (descriptor.reference.tag) {
      case rt::ValueTag::StackRef: {
        auto addr = descriptor.reference.as_ref.to_data;
        return (uint8_t*)&stack_.stack_area_[addr];
      }

      case rt::ValueTag::InstrRef: {
        auto addr = descriptor.reference.as_ref.to_instr;
        auto chunk = program_text_->GetTextSection(addr.chunk_no);
        FMT_ASSERT(chunk.length >= addr.instr_no, "");
        return &chunk.text[addr.instr_no];
      }

      case rt::ValueTag::HeapRef:
      case rt::ValueTag::StaticRef:
        FMT_ASSERT(false, "Unimplemented!");

      default:
        FMT_ASSERT(false, "Unreachable!");
    }
  }

  // I want to see it an undicriminated array of bytes
  // But I also want it to have some structure

 private:
  uint8_t* memory_;

  std::vector<MemAccess> access_log_;

  VmStack stack_;
  debug::StackPrinter stack_printer_{stack_};

  std::optional<ElfFile> program_text_;
};

}  // namespace vm::memory
