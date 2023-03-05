#pragma once

#include <lex/scanner.hpp>

#include <variant>
#include <cstddef>

namespace lex {

//////////////////////////////////////////////////////////////////////

struct Token {
  using SemInfo = std::variant<  //
      std::monostate,            //
      std::string_view,          //
      long long,                 //
      double                     //
      >;

  Token(TokenType type, Location start, SemInfo sem_info = {})
      : type{type}, location{start}, sem_info{sem_info} {
  }

  static Token UnitToken(Location loc) {
    return Token(TokenType::UNIT, loc);
  }

  Token() = default;

  operator std::string_view() {
    return GetName();
  }

  std::string_view GetName() const {
    FMT_ASSERT(type == TokenType::IDENTIFIER,
               "Requesting the name of non-identifier");
    return std::get<std::string_view>(sem_info);
  }

  TokenType type;

  Location location;

  SemInfo sem_info;
};

//////////////////////////////////////////////////////////////////////

}  // namespace lex
