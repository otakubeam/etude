#include <vm/elf_file.hpp>

namespace vm {

ElfFile::ElfFile(TextSection text, SymtabEntry symtab_sections,
                 std::vector<RelocationEntry> relocations,
                 std::vector<debug::DebugInfo> DIEs)
    : text_sections_{std::move(text)},
      symtab_sections_{{symtab_sections}},
      relocations_{std::move(relocations)},
      DIEs_{std::move(DIEs)} {
}

//////////////////////////////////////////////////////////////////////

auto ElfFile::FindEntryPoint() -> std::optional<rt::InstrReference> {
  for (auto& symbol : symtab_sections_) {
    if (symbol.name == "main") {
      return rt::InstrReference{
          .chunk_no = symbol.text_section_no,
      };
    }
  }
  return std::nullopt;
}

//////////////////////////////////////////////////////////////////////

void ElfFile::operator+=(ElfFile&& other) {
  MergeText(other.text_sections_);

  MergeSymtab(other.symtab_sections_);

  MergeRelocations(other.relocations_);

  LocateAll();
}

//////////////////////////////////////////////////////////////////////

void ElfFile::LocateAll() {
  for (auto& reloc : relocations_) {
    for (size_t i = 0; i < symtab_sections_.size(); i++) {
      auto& symbol = symtab_sections_.at(i);

      if (reloc.name == symbol.name) {
        Patch(reloc, symbol);

        DropRelocationEntry(reloc);

        i = 0;  // reset searching for symbols
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

void ElfFile::DropRelocationEntry(RelocationEntry& reloc) {
  reloc = std::move(relocations_.back());
  relocations_.pop_back();
}

void ElfFile::Patch(RelocationEntry reloc, SymtabEntry symbol) {
  auto patch = rt::InstrReference{.chunk_no = symbol.text_section_no};
  memcpy(GetOffsetToPatch(reloc), &patch, sizeof(rt::InstrReference));
}

auto ElfFile::GetOffsetToPatch(RelocationEntry reloc) -> uint8_t* {
  auto fn_entry = text_sections_.at(reloc.text_section_no);
  return &fn_entry.text[reloc.offset_to_patch];
}

//////////////////////////////////////////////////////////////////////

void ElfFile::MergeText(std::vector<TextSection> sections) {
  for (auto section : sections) {
    text_sections_.push_back(section);
  }
}

//////////////////////////////////////////////////////////////////////

void ElfFile::MergeSymtab(std::vector<SymtabEntry> entries) {
  auto initial_size = symtab_sections_.size();

  for (auto entry : entries) {
    entry.text_section_no += initial_size;
    symtab_sections_.push_back(entry);
  }
}

//////////////////////////////////////////////////////////////////////

void ElfFile::MergeRelocations(std::vector<RelocationEntry> entries) {
  auto initial_size = relocations_.size();

  for (auto entry : entries) {
    entry.text_section_no += initial_size;
    relocations_.push_back(entry);
  }
}

//////////////////////////////////////////////////////////////////////

}  // namespace vm
