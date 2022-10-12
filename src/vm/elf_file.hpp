#pragma once

#include <vm/rt/primitive.hpp>

#include <vm/instr_type.hpp>

#include <cstdlib>
#include <vector>
#include <map>

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

  uint16_t text_section_no = 0;
};

//////////////////////////////////////////////////////////////////////

struct RelocationEntry {
  std::string name;

  // Warn about missing initializers
  uint16_t text_section_no;
  uint16_t offset_to_patch;
};

//////////////////////////////////////////////////////////////////////

namespace debug {
class Disassembler;
class Debugger;
}  // namespace debug

class ElfFile {
 public:
  friend class debug::Disassembler;
  friend class debug::Debugger;

  ElfFile(TextSection text, SymtabEntry symtab_section,
          std::vector<RelocationEntry> relocation,
          std::map<uint16_t, debug::DebugInfo> DIEz);

  auto FindEntryPoint() -> std::optional<rt::InstrReference>;

  void operator+=(ElfFile&& other);

  auto GetTextSection(uint16_t no) -> TextSection {
    return text_sections_.at(no);
  }

 private:
  void LocateAll();

  void DropRelocationEntry(RelocationEntry& reloc);

  void Patch(RelocationEntry reloc, SymtabEntry symbol);

  auto GetOffsetToPatch(RelocationEntry reloc) -> uint8_t*;

  using SectionDIE = std::map<uint16_t, debug::DebugInfo>;
  void MergeDIEs(std::vector<SectionDIE> DIEs);

  void MergeText(std::vector<TextSection> sections);

  void MergeSymtab(std::vector<SymtabEntry> entries);

  void MergeRelocations(std::vector<RelocationEntry> entries);

 private:
  std::vector<TextSection> text_sections_;
  // TODO: std::vector<DataSection> data_sections;

  // Symbol information
  std::vector<SymtabEntry> symtab_sections_;
  std::vector<RelocationEntry> relocations_;

  // Debugging information
  std::vector<SectionDIE> DIEs_;
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm
