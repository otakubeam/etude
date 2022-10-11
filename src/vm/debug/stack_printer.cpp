#include <vm/debug/stack_printer.hpp>

namespace vm::debug {

////////////////////////////////////////////////////////////////////

StackPrinter::StackPrinter(memory::VmStack& stack) : stack_{stack} {
}

////////////////////////////////////////////////////////////////////

void StackPrinter::Print() {
  fmt::print("{}", Format());
}

////////////////////////////////////////////////////////////////////

std::string StackPrinter::Format() {
  fmt::memory_buffer buf;

  FormatHeader(buf);

  FormatStackCells(buf);

  fmt::format_to(std::back_inserter(buf), "\n");

  FormatRegisters(buf);

  fmt::format_to(std::back_inserter(buf), "\n\n");

  return fmt::to_string(buf);
}

std::string DotCell(rt::PrimitiveValue cell) {
  switch (cell.tag) {
    case rt::ValueTag::Int:
      return fmt::format("{}", cell.as_int);
    case rt::ValueTag::Bool:
      return fmt::format("{}", cell.as_bool);
    case rt::ValueTag::Char:
      return fmt::format("{}", cell.as_char);
    case rt::ValueTag::Unit:
      return fmt::format("unit");
    case rt::ValueTag::InstrRef:
      return rt::FormatInstrRef(cell.as_ref.to_instr);
    case rt::ValueTag::StackRef:
      return fmt::format("s{}", (cell.as_ref.to_data));
    default:
      return "<unsupported>";
  }
}

std::string StackPrinter::ToDot() {
  fmt::memory_buffer buf;

  std::string strucure;
  for (size_t i = 0; i < 32; i++) {
    strucure += fmt::format("<s{}> {} |", i, DotCell(stack_.stack_area_[i]));
  }

  fmt::format_to(std::back_inserter(buf),
                 "digraph G {{ rankdir=LR; "
                 "node [shape=record,width=.1,height=.1];"
                 "sp; fp; "
                 "stack [label = \" {} \"];\n",
                 strucure);

  fmt::format_to(std::back_inserter(buf), "sp -> stack:s{};\n", stack_.sp_);
  fmt::format_to(std::back_inserter(buf), "fp -> stack:s{};\n", stack_.fp_);

  for (size_t i = 0; i < stack_.sp_; i++) {
    if (stack_.stack_area_[i].tag == rt::ValueTag::StackRef) {
      fmt::format_to(std::back_inserter(buf), "stack:s{} -> stack:s{};\n", i,
                     stack_.stack_area_[i].as_ref.to_data);
    }
  }

  fmt::format_to(std::back_inserter(buf), "}}\n");

  return fmt::to_string(buf);
}

////////////////////////////////////////////////////////////////////

void StackPrinter::FormatHeader(fmt::memory_buffer& buf) {
  fmt::format_to(std::back_inserter(buf), bold, "[!] Stack:\n");

  int left = GetStartingPoint();

  for (int i = left; i < left + print_size; i++) {
    fmt::format_to(std::back_inserter(buf), bold, "{}\t", i);
  }

  fmt::format_to(std::back_inserter(buf), "\n");
}

////////////////////////////////////////////////////////////////////

void StackPrinter::FormatStackCells(fmt::memory_buffer& buf) {
  int left = GetStartingPoint();

  for (int i = left; i < left + print_size; i++) {
    FormatOneCell(buf, i);
  }
}

////////////////////////////////////////////////////////////////////

void StackPrinter::FormatRegisters(fmt::memory_buffer& buf) {
  int left = GetStartingPoint();

  for (int i = left; i < left + print_size; i++) {
    if (i == (int)stack_.sp_) {
      fmt::format_to(std::back_inserter(buf), italic, "sp\t");
    } else if (i == (int)stack_.fp_) {
      fmt::format_to(std::back_inserter(buf), italic, "fp\t");
    } else {
      fmt::format_to(std::back_inserter(buf), "  \t");
    }
  }
}

////////////////////////////////////////////////////////////////////

void StackPrinter::FormatOneCell(fmt::memory_buffer& buf, size_t index) {
  auto& cell = stack_.stack_area_[index];
  auto& an = annotations_.at(index);

  // Choose color

  if (index >= stack_.sp_) {
    an.style = fg(fmt::color::dim_gray);
  }

  if (index == stack_.fp_) {
    an.style = fg(fmt::color::light_green);
  }

  if (index + 1 == stack_.fp_) {
    an.style = fg(red) | bold;
  }

  // Format the contents of the cell

  switch (cell.tag) {
    case rt::ValueTag::Int:
      fmt::format_to(std::back_inserter(buf), an.style, "{}\t", cell.as_int);
      break;
    case rt::ValueTag::Bool:
      fmt::format_to(std::back_inserter(buf), an.style, "{}\t", cell.as_bool);
      break;
    case rt::ValueTag::Char:
      fmt::format_to(std::back_inserter(buf), an.style, "{}\t", cell.as_char);
      break;
    case rt::ValueTag::Unit:
      fmt::format_to(std::back_inserter(buf), an.style, "unit\t");
      break;
    case rt::ValueTag::InstrRef:
      fmt::format_to(std::back_inserter(buf), an.style, "{}\t",
                     rt::FormatInstrRef(cell.as_ref.to_instr));
      break;
    case rt::ValueTag::StackRef:
      fmt::format_to(std::back_inserter(buf), an.style, "s{}\t",
                     (cell.as_ref.to_data));
      break;
    default:
      break;
  }

  if (index == stack_.sp_) {
    an.style = fg(fmt::color::golden_rod);
  }
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::debug
