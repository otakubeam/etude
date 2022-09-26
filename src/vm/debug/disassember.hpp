#pragma once

#include <vm/elf_file.hpp>

#include <vm/decoder.hpp>

namespace vm::debug {

// Should I also make a readelf?
class Disassembler {
 public:
  void Disassemble(ElfFile& file) {
    for (auto symbol : file.symtab_sections_) {
      fmt::print("Disasm of <{}>, text section number {} in file {}\n\n",
                 symbol.name, symbol.text_section_no, (void*)&file);

      Disassemble(file.text_sections_[symbol.text_section_no]);

      // End line
      fmt::print("\n");
    }

    PrintRelocations(file);
  }

  void Decode(uint8_t*& instr) {
    auto type_bytes = FormatNBytes(1, instr);

    auto type = Decoder::DecodeType(instr);

    fmt::print("\t{:>3}:\t{:<12}\t {}", offset, FormatInstrType(type),
               type_bytes);

    switch (type) {
      case InstrType::PUSH_VALUE: {
        auto val_bytes = FormatNBytes(sizeof(rt::PrimitiveValue), instr);
        auto value = Decoder::DecodeValue(instr);

        fmt::print("{:<20} {}\n", val_bytes, FormatPrimitiveValue(*value));

        break;
      }

      case InstrType::CALL_FN: {
        auto instr_bytes = FormatNBytes(sizeof(rt::InstrReference), instr);
        auto ref = Decoder::DecodeReference(instr);

        fmt::print("{:<20} {}\n", instr_bytes, FormatInstrRef(*ref));

        break;
      }

      case InstrType::NATIVE_CALL:
      case InstrType::FIN_CALL: {
        auto instr_bytes = FormatNBytes(1, instr);

        fmt::print("{:<20} {}\n", instr_bytes, uint8_t(*instr));

        instr += 1;
        break;
      }

      case InstrType::JUMP:
      case InstrType::JUMP_IF_FALSE:
      case InstrType::GET_ARG:
      case InstrType::GET_LOCAL:
      case InstrType::LOAD:
      case InstrType::STORE:
        FMT_ASSERT(false, "Unreachable!");

      case InstrType::ADD:
      case InstrType::SUBTRACT:
      case InstrType::CMP_EQ:;
      case InstrType::CMP_LESS:;
      case InstrType::PUSH_TRUE:;
      case InstrType::PUSH_FALSE:;
      case InstrType::INDIRECT_CALL:;
      case InstrType::RET_FN:
      case InstrType::POP_STACK:
        fmt::print("\n");
    }
  }

 private:
  void Disassemble(TextSection text) {
    auto instr = text.text;
    auto text_end = text.text + text.length;
    while (instr < text_end) {
      offset = instr - text.text;
      Decode(instr);
    }
  }

  void PrintRelocations(ElfFile& file) {
    fmt::print("Relocation entries of this file:\n");

    for (auto reloc : file.relocations_) {
      fmt::print("\tsymbol {} at offset {} of section {}\n", reloc.name,
                 reloc.offset_to_patch, reloc.text_section_no);
    }

    fmt::print("\n");
  }

  // template <size_t n>
  std::string FormatNBytes(size_t n, uint8_t* instr) {
    if (n == 0) {
      return "";
    }
    return fmt::format("{:02x} ", *instr) + FormatNBytes(n - 1, instr + 1);
  }

 private:
  uint16_t offset = 0;
};

}  // namespace vm::debug
