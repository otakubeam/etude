#pragma once

#include <lex/token_type.hpp>
#include <lex/location.hpp>

#include <fmt/core.h>

#include <filesystem>
#include <iostream>
#include <istream>
#include <vector>
#include <string_view>

namespace lex {

//////////////////////////////////////////////////////////////////////

class Scanner {
 public:
  Scanner(std::istream& source) : source_{source} {
    InitBuffer();
    FetchNextSymbol();
  }

  void InitBuffer() {
    constexpr size_t INITIAL_BUF_SIZE = 2000;
    buffer_.reserve(INITIAL_BUF_SIZE);
  }

  void MoveRight() {
    switch (CurrentSymbol()) {
      case '\n':
        location_.columnno = 0;

        // Push the previous line to the vector
        lines_.push_back(std::string_view{
            lines_.back().end(),              // starting from the last line
            std::string_view{buffer_}.end(),  // to the end of the buffer
        });

        location_.lineno += 1;

        break;

      case EOF:
        // FMT_ASSERT(false, "\nReached EOF\n");
        break;

      default:
        location_.columnno += 1;
    }

    FetchNextSymbol();
  }

  template <typename F>
  std::string_view ViewWhile() {
    auto start_pos = buffer_.begin() + buffer_.size() - 1;
    auto count = 0;

    // For example:
    //   1. while not " char
    //   2. while alphanumeric

    while (F(CurrentSymbol())) {
      count += 1;
      MoveRight();
    }

    return std::string_view(start_pos, start_pos + count);
  }

  void MoveNextLine() {
    while (CurrentSymbol() != '\n') {
      MoveRight();
    }

    // Finally, move to the next line
    MoveRight();
  }

  char CurrentSymbol() {
    return symbol_;
  }

  char PeekNextSymbol() {
    return source_.peek();
  }

  Location GetLocation() const {
    return location_;
  }

 private:
  void FetchNextSymbol() {
    symbol_ = source_.get();

    if (symbol_ & std::istream::eofbit) {
      return;
    }

    buffer_.push_back(symbol_);
  }

 private:
  std::istream& source_;

  std::vector<std::string_view> lines_;

  std::string buffer_;

  Location location_;

  char symbol_;
};

//////////////////////////////////////////////////////////////////////

}  // namespace lex
