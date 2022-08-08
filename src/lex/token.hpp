#pragma once

#include <lex/scanner.hpp>

#include <variant>
#include <cstddef>

namespace lex {

//////////////////////////////////////////////////////////////////////

struct Token {
  // TODO: remove bool from SemInfo
  using SemInfo = std::variant<  //
      std::monostate,            //
      std::string,               //
      bool,                      //
      int                        //
      >;

  Token(TokenType type,  //
        Location start,  //
        SemInfo sem_info = {})
      : type{type}, tk_loc{start}, sem_info{sem_info} {
  }

  Token() = default;

  std::string GetName() const {
    FMT_ASSERT(type == TokenType::IDENTIFIER,
               "Requesting the name of non-identifier");
    return std::get<std::string>(sem_info);
  }

  TokenType type;

  Location tk_loc;

  SemInfo sem_info = {};
};

//////////////////////////////////////////////////////////////////////

}  // namespace lex
