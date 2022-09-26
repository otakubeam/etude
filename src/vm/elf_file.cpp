#include <vm/elf_file.hpp>

namespace vm {

ElfFile::ElfFile(TextSection text, SymtabEntry symtab_section,
                 std::vector<RelocationEntry> relocation,
                 std::vector<debug::DebugInfo> DIEz)
    : text_sections{std::move(text)},
      symtab_section{{symtab_section}},
      relocations{std::move(relocation)},
      DIEs{std::move(DIEz)} {
}

//////////////////////////////////////////////////////////////////////

auto ElfFile::FindEntryPoint() -> std::optional<rt::InstrReference> {
  for (auto& symbol : symtab_section) {
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
  MergeText(other.text_sections);

  MergeSymtab(other.symtab_section);

  MergeRelocations(other.relocations);

  LocateAll();
}

//////////////////////////////////////////////////////////////////////

void ElfFile::LocateAll() {
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

//////////////////////////////////////////////////////////////////////

void ElfFile::DropRelocationEntry(RelocationEntry& reloc) {
  reloc = std::move(relocations.back());
  relocations.pop_back();
}

void ElfFile::Patch(RelocationEntry reloc, SymtabEntry symbol) {
  auto patch = rt::InstrReference{.chunk_no = symbol.text_section_no};
  memcpy(GetOffsetToPatch(reloc), &patch, sizeof(rt::InstrReference));
}

auto ElfFile::GetOffsetToPatch(RelocationEntry reloc) -> uint8_t* {
  auto fn_entry = text_sections.at(reloc.text_section_no);
  return &fn_entry.text[reloc.offset_to_patch];
}

//////////////////////////////////////////////////////////////////////

void ElfFile::MergeText(std::vector<TextSection> sections) {
  for (auto section : sections) {
    text_sections.push_back(section);
  }
}

//////////////////////////////////////////////////////////////////////

void ElfFile::MergeSymtab(std::vector<SymtabEntry> entries) {
  auto initial_size = symtab_section.size();

  for (auto entry : entries) {
    entry.text_section_no += initial_size;
    symtab_section.push_back(entry);
  }
}

//////////////////////////////////////////////////////////////////////

void ElfFile::MergeRelocations(std::vector<RelocationEntry> entries) {
  auto initial_size = relocations.size();

  for (auto entry : entries) {
    entry.text_section_no += initial_size;
    relocations.push_back(entry);
  }
}

//////////////////////////////////////////////////////////////////////

}  // namespace vm
