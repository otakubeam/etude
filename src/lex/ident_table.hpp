#pragma once

#include <lex/token_type.hpp>

#include <string>
#include <map>

namespace lex {

class IdentTable {
 public:
  IdentTable() {
    Populate();
  }

  TokenType LookupOrInsert(const std::string& lexeme) {
    if (!map_.contains(lexeme)) {
      map_.insert({lexeme, TokenType::IDENTIFIER});
    }
    return map_[lexeme];
  }

 private:
  void Populate() {
    map_.insert({"return", TokenType::RETURN});
    map_.insert({"yield", TokenType::YIELD});
    map_.insert({"false", TokenType::FALSE});
    map_.insert({"else", TokenType::ELSE});
    map_.insert({"true", TokenType::TRUE});
    map_.insert({"var", TokenType::VAR});
    map_.insert({"fun", TokenType::FUN});
    map_.insert({"for", TokenType::FOR});
    map_.insert({"if", TokenType::IF});
  }

 private:
  std::map<std::string, TokenType> map_;
};

}  // namespace lex
