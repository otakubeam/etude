#pragma once

#include <lex/scanner.hpp>

#include <variant>
#include <cstddef>
#include <string_view>

namespace lex {

//////////////////////////////////////////////////////////////////////

struct Token {
  using SemInfo = std::variant<  //
      std::monostate,            //
      std::string_view,          //
      int                        //
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

  // Location of the symbol right after the token. Zero-indexed. 
  Location location;

  SemInfo sem_info;

  // Best if the lexer would store lengths. That would be lengths
  //   in the file. Whereas right now we decide that indirectly,
  //   by the values stored. Making assumptions on how it was presented
  //   in the original source file.
  // Not only that, but such switch statements produce another point
  //   where a programmer would need to touch code, if one adds an
  //   enum item. If these are spread out, the task of adding
  //   functionality becomes more difficult. Programmer has to get
  //   context of the place one is touching. If it's only here, it's
  //   alright.
  // Also, need to implement tests for this function.
  size_t length() const {
    using namespace std::string_view_literals;

    switch (type) {
      case TokenType::NUMBER: {
        // We expect here that number is in decimal base (which may be wrong, it
        //   is useful to have hexadecimal numbers, at the very
        //   least) and that the number doesn't have leading zeroes.
        // If that doesn't hold, computed lengths is not the same
        //   that length in file. But we need length in file..
        return std::to_string(std::get<int>(sem_info)).size();
      }
      case TokenType::CHAR: {
        // 'c'
        return 3;
      }
      case TokenType::STRING: {
        // "abcd"
        return 2 + std::get<std::string_view>(sem_info).size();
      }
      case TokenType::IDENTIFIER: {
        // In fun main = { } main is an ident.
        return std::get<std::string_view>(sem_info).size();
      }

      // Example is in constexpr expressions below this note.

      case TokenType::UNIT:     return "()"sv.size();

      case TokenType::TRUE:     return "true"sv.size();
      case TokenType::FALSE:    return "false"sv.size();

      case TokenType::PLUS:     return "+"sv.size();
      case TokenType::MINUS:    return "-"sv.size();
      case TokenType::DIV:      return "/"sv.size();

      case TokenType::PLUS_EQ:  return "+="sv.size();
      case TokenType::MINUS_EQ: return "-="sv.size();
      case TokenType::STAR_EQ:  return "*="sv.size();
      case TokenType::DIV_EQ:   return "/="sv.size();

      case TokenType::ATTRIBUTE: {
        // ???????
        return 0;
      }
      case TokenType::EXPORT: return "export"sv.size();
      case TokenType::EXTERN: {
        // ??????
        return 0;
      }

      case TokenType::ASSIGN:       return "="sv.size();
      case TokenType::EQUALS:       return "=="sv.size();
      case TokenType::NOT_EQ:       return "!="sv.size();
      case TokenType::LT:           return "<"sv.size();
      case TokenType::LE:           return "<="sv.size();
      case TokenType::GT:           return ">"sv.size();
      case TokenType::GE:           return ">="sv.size();

      case TokenType::LEFT_PAREN:   return "("sv.size();
      case TokenType::RIGHT_PAREN:  return ")"sv.size();

      // CURLY
      case TokenType::LEFT_CBRACE:  return "{"sv.size();
      case TokenType::RIGHT_CBRACE: return "}"sv.size();

      // SQUARE
      case TokenType::LEFT_SBRACE:  return "["sv.size();
      case TokenType::RIGHT_SBRACE: return "]"sv.size();

      case TokenType::NOT:          return "!"sv.size();

      case TokenType::ADDR:         return "&"sv.size(); // TODO: check if I'm right.
      
      case TokenType::STAR:         return "*"sv.size();

      case TokenType::ARROW:        return "->"sv.size();

      case TokenType::ARROW_CAST:   return "~>"sv.size();

      case TokenType::NEW:          return "new"sv.size();

      case TokenType::FUN:          return "fun"sv.size();

      case TokenType::DOT:          return "dot"sv.size();
      
      case TokenType::COMMA:        return ","sv.size();

      case TokenType::VAR:          return "var"sv.size();
      case TokenType::TYPE:         return "type"sv.size();
      case TokenType::TRAIT:        return "trait"sv.size();
      case TokenType::STRUCT:       return "struct"sv.size();
      case TokenType::SUM:          return "sum"sv.size();
      case TokenType::UNION:        return "union"sv.size();
      
      case TokenType::OF:           return "of"sv.size();
      case TokenType::FOR:          return "for"sv.size();
      case TokenType::IMPL:         return "impl"sv.size();
      case TokenType::UNDERSCORE:   return "_"sv.size();

      case TokenType::TY_INT:       return "int"sv.size();
      case TokenType::TY_BOOL:      return "bool"sv.size();
      case TokenType::TY_CHAR:      return "char"sv.size();
      case TokenType::TY_UNIT:      return "()"sv.size();
      case TokenType::TY_STRING:    return "string"sv.size();

      case TokenType::IF:           return "if"sv.size();
      case TokenType::MATCH:        return "match"sv.size();
      case TokenType::BIT_OR:       return "|"sv.size();
      case TokenType::THEN:         return "then"sv.size();
      case TokenType::ELSE:         return "else"sv.size();

      case TokenType::COLON:        return ":"sv.size();
      case TokenType::SEMICOLON:    return ";"sv.size();
      case TokenType::RETURN:       return "return"sv.size();
      case TokenType::YIELD:        return "yield"sv.size();

      case TokenType::TOKEN_EOF:    return 0;

      default:                      __builtin_unreachable();
    }
  }
};

//////////////////////////////////////////////////////////////////////

}  // namespace lex
