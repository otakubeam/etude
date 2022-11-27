#pragma once

#include <fmt/core.h>

#include <cstddef>
#include <string>

namespace lex {

struct Location {
  size_t lineno = 0;
  size_t columnno = 0;

  std::string Format() const {
    return fmt::format("line = {}, column = {}",  //
                       lineno + 1, columnno + 1);
  }
};

}  // namespace lex
