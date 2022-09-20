#pragma once

#include <lex/scanner.hpp>

#include <variant>
#include <cstddef>

namespace lex {

//////////////////////////////////////////////////////////////////////

struct Token {
  using SemInfo = std::variant<  //
      std::monostate,            //
      std::string,               //
      int                        //
      >;

  Token(TokenType type,  //
        Location start,  //
        SemInfo sem_info = {})
      : type{type}, location{start}, sem_info{sem_info} {
  }

  Token() = default;

  std::string GetName() const {
    FMT_ASSERT(type == TokenType::IDENTIFIER,
               "Requesting the name of non-identifier");
    return std::get<std::string>(sem_info);
  }

  TokenType type;

  Location location;

  SemInfo sem_info = {};
};

//////////////////////////////////////////////////////////////////////

}  // namespace lex
