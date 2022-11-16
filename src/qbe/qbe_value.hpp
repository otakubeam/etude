#pragma once

#include <fmt/core.h>
#include <string>

namespace qbe {

struct Value {
  enum {
    GLOBAL,
    TEMPORARY,
    CONST_INT,
  } tag;

  std::string Emit() const {
    switch (tag) {
      case GLOBAL:
        return fmt::format("${}", name);
      case TEMPORARY:
        return fmt::format("%.{}", id);
      case CONST_INT:
        return fmt::format("{}", value);
      default:
        std::abort();
    }
  }

  std::string_view name{};
  int value = 0;
  int id = 0;
};

}  // namespace qbe
