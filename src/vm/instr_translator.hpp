#pragma once

#include <vm/codegen/fat_instr.hpp>
#include <vm/instr_type.hpp>
#include <vm/elf_file.hpp>

#include <lex/location.hpp>

#include <vector>

namespace vm {

class InstrTranslator {
 public:
  InstrTranslator(std::string name) : fn_name{std::move(name)} {
  }

  // Usage:
  // std::move(InstrTranslator).Finalize();

  ElfFile Finalize() && {
    return ElfFile{
        TextSection{.text = bytecode_, .length = length_},
        SymtabEntry{.name = fn_name},
        std::move(relocations_),
        std::move(DIEs_),
    };
  }

  void TranslateInstruction(const codegen::FatInstr& instr) {
    EmitType(instr.type);

    DIEs_.push_back(instr.debug_info);

    switch (instr.type) {
      case InstrType::CALL_FN:
        relocations_.push_back(RelocationEntry{
            .name = instr.fn_name,
            .text_section_no = 0,
            .offset_to_patch = length_,
        });
        // Mark it
        EmitInstrReference({.chunk_no = 0xffff, .instr_no = 0xffff});
        break;

      case InstrType::PUSH_VALUE:
        EmitPrimitiveValue(instr.value);
        break;

      case InstrType::JUMP_IF_FALSE:
      case InstrType::ADD_ADDR:
      case InstrType::GET_AT_FP:
      case InstrType::JUMP:
        EmitHalf(instr.offset);
        break;

      case InstrType::NATIVE_CALL:
      case InstrType::FIN_CALL:
      case InstrType::STORE:
        Emit(instr.arg);
        break;

      case InstrType::INDIRECT_CALL:
      case InstrType::LOAD:
      case InstrType::CMP_EQ:
      case InstrType::CMP_LESS:
      case InstrType::RET_FN:
      case InstrType::POP_STACK:
      case InstrType::ADD:
      case InstrType::PUSH_FP:
      case InstrType::SUBTRACT:
      case InstrType::PUSH_TRUE:
      case InstrType::PUSH_FALSE:
      case InstrType::PUSH_UNIT:;
        // No-op
    }
  }

  size_t Count() {
    return DIEs_.size();
  }

 private:
  void Emit(uint8_t byte) {
    bytecode_[length_] = byte;
    length_ += 1;
  }

  void EmitType(vm::InstrType type) {
    Emit(uint8_t(type));
  }

  void EmitHalf(int16_t half) {
    memcpy(bytecode_ + length_, &half, 2);
    length_ += 2;
  }

  void EmitInstrReference(rt::InstrReference ref) {
    memcpy(bytecode_ + length_, &ref, sizeof(rt::InstrReference));
    length_ += sizeof(rt::InstrReference);
  }

  void EmitPrimitiveValue(rt::PrimitiveValue value) {
    memcpy(bytecode_ + length_, &value, sizeof(rt::PrimitiveValue));
    length_ += sizeof(rt::PrimitiveValue);
  }

 private:
  std::string fn_name;

  uint8_t* bytecode_ = new uint8_t[65536]();
  uint16_t length_ = 0;

  std::vector<debug::DebugInfo> DIEs_;

  std::vector<RelocationEntry> relocations_;
};

}  // namespace vm