#include <vm/debug/disassember.hpp>

namespace vm::debug {

////////////////////////////////////////////////////////////////////

void Disassembler::Disassemble(ElfFile& file) {
  for (auto symbol : file.symtab_sections_) {
    fmt::print("Disasm of <{}>, text section number {} in file {}\n\n",
               symbol.name, symbol.text_section_no, (void*)&file);

    Disassemble(file.text_sections_[symbol.text_section_no]);

    // End line
    fmt::print("\n");
  }

  FormatRelocations(file);
}

////////////////////////////////////////////////////////////////////

std::string Disassembler::FormatInstruction(uint8_t*& instr) {
  auto type_bytes = FormatNBytes(1, instr);

  auto type = Decoder::DecodeType(instr);

  auto format_instr_type =
      fmt::format("{:<12}\t {}", FormatInstrType(type), type_bytes);

  // Looks like this:

  // push_value       00 00 64 00 00 00       Value [tag:Int, value 100]

  return format_instr_type + FormatArguments(type, instr);
}

////////////////////////////////////////////////////////////////////

void Disassembler::Disassemble(TextSection text) {
  auto instr = text.text;
  auto text_end = text.text + text.length;

  while (instr < text_end) {
    offset = instr - text.text;
    fmt::print("{}{}", FormatOffset(), FormatInstruction(instr));
  }
}

////////////////////////////////////////////////////////////////////

std::string Disassembler::FormatArguments(InstrType type, uint8_t*& instr) {
  std::string bytes, pretty_print;

  switch (type) {
    case InstrType::PUSH_VALUE: {
      bytes = FormatNBytes(sizeof(rt::PrimitiveValue), instr);
      pretty_print = rt::FormatPrimitiveValue(*Decoder::DecodeValue(instr));
      break;
    }

    case InstrType::CALL_FN: {
      bytes = FormatNBytes(sizeof(rt::InstrReference), instr);
      pretty_print = rt::FormatInstrRef(*Decoder::DecodeReference(instr));
      break;
    }

    case InstrType::NATIVE_CALL:
    case InstrType::STORE:
    case InstrType::FIN_CALL: {
      bytes = FormatNBytes(1, instr);
      pretty_print = fmt::format("{}", *(instr));
      instr += 1;
      break;
    }

    case InstrType::JUMP_IF_FALSE:
    case InstrType::ADD_ADDR:
    case InstrType::GET_AT_FP:
    case InstrType::JUMP: {
      bytes = FormatNBytes(2, instr);
      pretty_print = fmt::format("{}", (int16_t)*instr);
      instr += 2;
      break;
    }

    case InstrType::LOAD:
    case InstrType::INDIRECT_CALL:
    case InstrType::ADD:
    case InstrType::SUBTRACT:
    case InstrType::CMP_EQ:
    case InstrType::CMP_LESS:
    case InstrType::PUSH_TRUE:
    case InstrType::PUSH_FALSE:
    case InstrType::RET_FN:
    case InstrType::POP_STACK:
      return "\n";
  }

  return fmt::format("{:<20} {}\n", bytes, pretty_print);
}

////////////////////////////////////////////////////////////////////

std::string Disassembler::FormatOffset() {
  return fmt::format("\t{:>3}:\t", offset);
}

////////////////////////////////////////////////////////////////////

std::string Disassembler::FormatRelocations(ElfFile& file) {
  auto result = fmt::format("Relocation entries of this file:\n");

  for (auto reloc : file.relocations_) {
    result +=
        fmt::format("\tsymbol {} at offset {} of section {}\n",  //
                    reloc.name, reloc.offset_to_patch, reloc.text_section_no);
  }

  return result + fmt::format("\n");
}

////////////////////////////////////////////////////////////////////

std::string Disassembler::FormatNBytes(size_t n, uint8_t* instr) {
  return (n == 0)
             ? ""
             : fmt::format("{:02x} ", *instr) + FormatNBytes(n - 1, instr + 1);
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::debug
