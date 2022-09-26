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
  uint16_t text_section_no;
};

//////////////////////////////////////////////////////////////////////

struct RelocationEntry {
  std::string name;

  uint16_t text_section_no;
  uint16_t offset_to_patch;
};

//////////////////////////////////////////////////////////////////////

struct ElfFile {
  std::vector<TextSection> text_sections;
  // std::vector<DataSection> data_sections;
  std::vector<SymtabEntry> symtab_section;
  std::vector<RelocationEntry> relocations;

  std::vector<debug::DebugInfo> DIEs;

  std::optional<rt::InstrReference> FindEntryPoint() {
    for (auto& symbol : symtab_section) {
      if (symbol.name == "main") {
        return rt::InstrReference{
            .chunk_no = symbol.text_section_no,
        };
      }
    }
    return std::nullopt;
  }

  void operator+=(ElfFile&& other) {
    MergeText(other.text_sections);

    MergeSymtab(other.symtab_section);

    MergeRelocations(other.relocations);

    LocateAll();
  }

  void LocateAll() {
    for (auto& reloc : relocations) {
      for (size_t i = 0; i < symtab_section.size(); i++) {
        auto& symbol = symtab_section.at(i);

        if (reloc.name == symbol.name) {
          Patch(reloc, symbol);

          DropRelocationEntry(reloc);

          i = 0;  // reset searching for symbols
        }
      }
    }
  }

  void DropRelocationEntry(RelocationEntry& reloc) {
    reloc = std::move(relocations.back());
    relocations.pop_back();
  }

  void Patch(RelocationEntry reloc, SymtabEntry symbol) {
    auto patch = rt::InstrReference{.chunk_no = symbol.text_section_no};
    memcpy(GetOffsetToPatch(reloc), &patch, sizeof(rt::InstrReference));
  }

  auto GetOffsetToPatch(RelocationEntry reloc) -> uint8_t* {
    auto fn_entry = text_sections.at(reloc.text_section_no);
    return &fn_entry.text[reloc.offset_to_patch];
  }

  void MergeText(std::vector<TextSection> sections) {
    for (auto section : sections) {
      text_sections.push_back(section);
    }
  }

  void MergeSymtab(std::vector<SymtabEntry> entries) {
    auto initial_size = symtab_section.size();

    for (auto entry : entries) {
      entry.text_section_no += initial_size;
      symtab_section.push_back(entry);
    }
  }

  void MergeRelocations(std::vector<RelocationEntry> entries) {
    auto initial_size = relocations.size();

    for (auto entry : entries) {
      entry.text_section_no += initial_size;
      relocations.push_back(entry);
    }
  }
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm
