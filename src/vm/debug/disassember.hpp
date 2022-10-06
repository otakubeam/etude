#pragma once

#include <vm/elf_file.hpp>

#include <vm/decoder.hpp>

namespace vm::debug {

////////////////////////////////////////////////////////////////////

class Disassembler {
 public:
  void Disassemble(ElfFile& file);

  static std::string FormatInstruction(uint8_t*& instr);

 private:
  void Disassemble(TextSection text);

  std::string FormatOffset();

  static std::string FormatArguments(InstrType type, uint8_t*& instr);

  std::string FormatRelocations(ElfFile& file);

  static std::string FormatNBytes(size_t n, uint8_t* instr);

 private:
  uint16_t offset = 0;
};

////////////////////////////////////////////////////////////////////

}  // namespace vm::debug
