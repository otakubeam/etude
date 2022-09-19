#include <vm/debug/stack_printer.hpp>

namespace vm::debug {

////////////////////////////////////////////////////////////////////

StackPrinter::StackPrinter(VmStack& stack) : stack_{stack} {
}

////////////////////////////////////////////////////////////////////

void StackPrinter::Print() {
  fmt::print("{}", Format());
}

////////////////////////////////////////////////////////////////////

std::string StackPrinter::Format() {
  if (!print_debug_info) {
    return "";
  }

  fmt::memory_buffer buf;

  FormatHeader(buf);

  FormatStackCells(buf);

  fmt::format_to(std::back_inserter(buf), "\n");

  for (int i = 0; i < 24; i++) {
    if (i == (int)stack_.sp_) {
      fmt::format_to(std::back_inserter(buf), italic, "sp\t");
    } else if (i == (int)stack_.fp_) {
      fmt::format_to(std::back_inserter(buf), italic, "fp\t");
    } else {
      fmt::format_to(std::back_inserter(buf), "  \t");
    }
  }

  fmt::format_to(std::back_inserter(buf), "\n");
  fmt::format_to(std::back_inserter(buf), "\n");

  return fmt::to_string(buf);
}

////////////////////////////////////////////////////////////////////

void StackPrinter::FormatHeader(fmt::memory_buffer& buf) {
  fmt::format_to(std::back_inserter(buf), bold, "[!] Stack:\n");

  for (int i = 0; i < 24; i++) {
    fmt::format_to(std::back_inserter(buf), bold, "{}\t", i);
  }

  fmt::format_to(std::back_inserter(buf), "\n");
}

void StackPrinter::FormatStackCells(fmt::memory_buffer& buf) {
  for (int i = 0; i < 24; i++) {
    FormatOneCell(buf, i);
  }
}

////////////////////////////////////////////////////////////////////

void StackPrinter::FormatOneCell(fmt::memory_buffer& buf, size_t index) {
  auto& cell = stack_.stack_.at(index);
  auto& an = annotations_.at(index);

  if (index >= stack_.sp_) {
    an.style = fg(fmt::color::dim_gray);
  }

  if (index == stack_.fp_) {
    an.style = fg(fmt::color::light_green);
  }

  if (index + 1 == stack_.fp_ || index + 2 == stack_.fp_) {
    an.style = fg(red) | bold;
  }

  fmt::format_to(std::back_inserter(buf), an.style, "{}\t", cell.as_int);

  if (index == stack_.sp_) {
    an.style = fg(fmt::color::golden_rod);
  }
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::debug
