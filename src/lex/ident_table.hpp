#pragma once

#include <lex/token_type.hpp>

#include <string>
#include <map>

#include <fmt/core.h>

namespace lex {

class IdentTable {
 public:
  IdentTable() {
    Populate();
  }

  // Use-of-string-view-for-map-lookup
  // https://stackoverflow.com/questions/35525777

  TokenType LookupWord(const std::string_view lexeme) {
    if (!map_.contains(lexeme)) {
      return TokenType::IDENTIFIER;
    }
    return map_.find(lexeme)->second;
  }

 private:
  void Populate() {
    map_.insert({"String", TokenType::TY_STRING});
    map_.insert({"Bool", TokenType::TY_BOOL});
    map_.insert({"Char", TokenType::TY_CHAR});
    map_.insert({"Unit", TokenType::TY_UNIT});
    map_.insert({"Int", TokenType::TY_INT});

    map_.insert({"return", TokenType::RETURN});
    map_.insert({"struct", TokenType::STRUCT});
    map_.insert({"export", TokenType::EXPORT});
    map_.insert({"extern", TokenType::EXTERN});
    map_.insert({"match", TokenType::MATCH});
    map_.insert({"trait", TokenType::TRAIT});
    map_.insert({"yield", TokenType::YIELD});
    map_.insert({"false", TokenType::FALSE});
    map_.insert({"then", TokenType::THEN});
    map_.insert({"impl", TokenType::IMPL});
    map_.insert({"else", TokenType::ELSE});
    map_.insert({"true", TokenType::TRUE});
    map_.insert({"unit", TokenType::UNIT});
    map_.insert({"type", TokenType::TYPE});
    map_.insert({"new", TokenType::NEW});
    map_.insert({"var", TokenType::VAR});
    map_.insert({"let", TokenType::LET});
    map_.insert({"fun", TokenType::FUN});
    map_.insert({"sum", TokenType::SUM});
    map_.insert({"for", TokenType::FOR});
    map_.insert({"if", TokenType::IF});
    map_.insert({"of", TokenType::OF});
  }

 private:
  // What-are-transparent-comparators
  // https://stackoverflow.com/questions/20317413

  std::map<std::string, TokenType, std::less<>> map_;
};

}  // namespace lex
