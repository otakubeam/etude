#pragma once

#include <vm/stack.hpp>
#include <vm/instr.hpp>

#include <fmt/color.h>

extern bool print_debug_info;

namespace vm::debug {

class StackPrinter {
 public:
  StackPrinter(VmStack& stack) : stack_{stack} {
  }

  struct AnnotatedSlot {
    enum class Type {
      EMPTY,
      IP,
      ARG,
      LOCAL,
    } type;

    fmt::text_style style = fg(fmt::color::black);
  };

  void Print() {
    fmt::print("{}", Format());
  }

  std::string Format() const {
    if (!print_debug_info) {
      return "";
    }

    fmt::memory_buffer buf;

    FormatHeader(buf);

    FormatStackCells(buf);

    fmt::format_to(std::back_inserter(buf), "\n");

    for (int i = 0; i < 16; i++) {
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

 private:
  void FormatHeader(fmt::memory_buffer& buf) const {
    fmt::format_to(std::back_inserter(buf), bold, "[!] Stack:\n");

    for (int i = 0; i < 16; i++) {
      fmt::format_to(std::back_inserter(buf), bold, "{}\t", i);
    }

    fmt::format_to(std::back_inserter(buf), "\n");
  }

  void FormatStackCells(fmt::memory_buffer& buf) const {
    for (int i = 0; i < 16; i++) {
      FormatOneCell(buf, i);
    }
  }

  void FormatOneCell(fmt::memory_buffer& buf, size_t index) const {
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

 private:
  VmStack& stack_;

  const static auto italic{fmt::emphasis::italic};
  const static auto bold{fmt::emphasis::bold};

  const static auto red{fmt::terminal_color::bright_red};
  const static auto cyan{fmt::color::cyan};

  mutable std::vector<AnnotatedSlot> annotations_{65536};
};

}  // namespace vm::debug
