#pragma once

#include <vm/instr_type.hpp>
#include <vm/stack_v2.hpp>

#include <fmt/color.h>

extern bool print_debug_info;

namespace vm::debug {

class StackPrinter {
 public:
  StackPrinter(VmStack& stack);

  struct AnnotatedSlot {
    fmt::text_style style = fg(fmt::color::black);
    std::string name;
  };

  void Print();
  std::string Format();

 private:
  void FormatHeader(fmt::memory_buffer& buf);
  void FormatStackCells(fmt::memory_buffer& buf);
  void FormatOneCell(fmt::memory_buffer& buf, size_t index);

 private:
  const static auto italic{fmt::emphasis::italic};
  const static auto bold{fmt::emphasis::bold};

  const static auto red{fmt::terminal_color::bright_red};
  const static auto cyan{fmt::color::cyan};

 private:
  const VmStack& stack_;
  std::vector<AnnotatedSlot> annotations_{65536};
};

}  // namespace vm::debug
