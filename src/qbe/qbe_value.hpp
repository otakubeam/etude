#pragma once

#include <fmt/core.h>
#include <string>

namespace qbe {

struct Value {
  enum {
    NONE,  // For debug
    PARAM,
    GLOBAL,
    TEMPORARY,
    CONST_INT,
  } tag;

  static Value None() {
    return {.tag = NONE};
  }

  Value AsPattern() {
    auto res = *this;
    res.tag = PARAM;
    return res;
  }

  std::string Emit() const {
    switch (tag) {
      case NONE:
        return "";
      case GLOBAL:
        return fmt::format("${}", name);
      case TEMPORARY:
      case PARAM:
        return fmt::format("%.{}", id);
      case CONST_INT:
        return fmt::format("{}", value);
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
