#pragma once

#include <fmt/core.h>

#include <cstddef>
#include <string>

#include <filesystem>

class Module;

namespace lex {

struct Location {
  Module* unit;
  size_t lineno = 0;
  size_t columnno = 0;

  std::string Format() const {
    return fmt::format("line = {}, column = {}",  //
                       lineno + 1, columnno + 1);
  }
};

}  // namespace lex
