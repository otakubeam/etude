#pragma once

#include <fmt/core.h>
#include <string>

namespace qbe {

struct Value {
  enum {
    NONE,  // For debug
    PARAM,
    GLOBAL,
    ADDRESS,
    TEMPORARY,
    CONST_INT,
  } tag;

  static Value None() {
    return {.tag = NONE};
  }

  std::string Emit() const {
    switch (tag) {
      case GLOBAL:
        return fmt::format("${}", name);
      case TEMPORARY:
      case PARAM:
        return fmt::format("%.{}", id);
      case CONST_INT:
        return fmt::format("{}", value);
      case ADDRESS:
        return fmt::format("{}", addr);
      default:
        std::abort();
    }
  }

  std::string_view aggregate_type{};
  std::string name{};
  size_t addr = 0;
  int value = 0;
  int id = 0;
};

}  // namespace qbe
