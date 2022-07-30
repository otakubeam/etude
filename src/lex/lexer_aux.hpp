#pragma once

#include <lex/token_type.hpp>

#include <optional>
#include <istream>

namespace lex {

inline bool IsWhitespace(char ch) {
  return ch == ' ' || ch == '\n' || ch == '\t';
}

}  // namespace lex
