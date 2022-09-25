#pragma once

#include <vm/rt/primitive.hpp>

#include <vm/instr_type.hpp>

#include <cstdlib>
#include <vector>

namespace vm {

//////////////////////////////////////////////////////////////////////

struct TextSection {
  uint8_t* text = nullptr;
  size_t length = 0;
};

//////////////////////////////////////////////////////////////////////

struct SymtabEntry {
  std::string name;

  // TODO: what about extern globals?
  // So, I need a entries of several types

  // Where it is defined
  size_t text_section_no;
};

//////////////////////////////////////////////////////////////////////

struct RelocationEntry {
  std::string name;

  size_t text_section_no;
  size_t offset_to_patch;
};

//////////////////////////////////////////////////////////////////////

struct ElfFile {
  std::vector<TextSection> text_sections;
  // std::vector<DataSection> data_sections;
  std::vector<SymtabEntry> symtab_section;
  std::vector<RelocationEntry> relocations;

  std::vector<debug::DebugInfo> DIEs;

  std::optional<size_t> FindMain() {
    for (auto& symbol : symtab_section) {
      if (symbol.name == "main") {
        return symbol.text_section_no;
      }
    }
    return std::nullopt;
  }
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm
