#pragma once

#include <vm/memory/stack_v2.hpp>

#include <vm/instr_type.hpp>

#include <fmt/color.h>

extern bool print_debug_info;

namespace vm::debug {

class StackPrinter {
 public:
  StackPrinter(memory::VmStack& stack);

  struct AnnotatedSlot {
    fmt::text_style style = fg(fmt::color::black);
    std::size_t dot_color = 1;
    std::string name;
    std::string type;
  };

  void Print();
  std::string Format();
  std::string ToDot();

 private:
  void FormatHeader(fmt::memory_buffer& buf);
  void FormatStackCells(fmt::memory_buffer& buf);
  void FormatRegisters(fmt::memory_buffer& buf);
  void FormatOneCell(fmt::memory_buffer& buf, size_t index);

  size_t GetStartingPoint() {
    return std::max(0, (int)stack_.fp_ - 5);
  }

 private:
  const static auto print_size = 20;

  const static auto italic{fmt::emphasis::italic};
  const static auto bold{fmt::emphasis::bold};

  const static auto red{fmt::terminal_color::bright_red};
  const static auto cyan{fmt::color::cyan};

 private:
  const memory::VmStack& stack_;
  std::vector<AnnotatedSlot> annotations_{1024};
};

}  // namespace vm::debug
