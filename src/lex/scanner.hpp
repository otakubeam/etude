#pragma once

#include <lex/token_type.hpp>
#include <lex/location.hpp>

#include <fmt/core.h>

#include <string_view>
#include <filesystem>
#include <iostream>
#include <istream>
#include <sstream>
#include <vector>
#include <span>

namespace lex {

struct InputFile {
  std::stringstream stream;
  std::filesystem::path abs_path;
};

//////////////////////////////////////////////////////////////////////

class Scanner {
 public:
  Scanner(lex::InputFile source) : source_{std::move(source)}, location_{nullptr, 0, 0} {
    InitBuffer();
    FetchNextSymbol();
  }

  void SetUnit(Module* unit) {
    location_.unit = unit;
  }

  void InitBuffer() {
    constexpr size_t INITIAL_BUF_SIZE = 20000;
    buffer_.reserve(INITIAL_BUF_SIZE);
  }

  void MoveRight() {
    switch (CurrentSymbol()) {
      case '\n':
        location_.columnno = 0;

        // Push the previous line to the vector
        lines_.push_back(std::string_view{
            PreviousLineEnd(),                // starting from the last line
            std::string_view{buffer_}.end(),  // to the end of the buffer
        });

        location_.lineno += 1;

        break;

      case EOF:
        break;

      default:
        location_.columnno += 1;
    }

    FetchNextSymbol();
  }

  template <auto F>
  std::string_view ViewWhile() {
    auto start_pos = buffer_.end() - 1;

    // For example:
    //   1. while not " char
    //   2. while alphanumeric

    while (F(CurrentSymbol())) {
      MoveRight();
    }

    return std::string_view(start_pos, buffer_.end() - 1);
  }

  void MoveNextLine() {
    while (CurrentSymbol() != '\n' && !source_.stream.eof()) {
      MoveRight();
    }

    // Finally, move to the next line
    MoveRight();
  }

  char CurrentSymbol() {
    return symbol_;
  }

  char PeekNextSymbol() {
    return source_.stream.peek();
  }

  Location GetLocation() const {
    return location_;
  }

  std::span<std::string_view> GetLines() {
    return lines_;
  }

  std::filesystem::path GetFileAbsPath() const {
    return source_.abs_path;
  }

 private:
  void FetchNextSymbol() {
    if (source_.stream.eof()) {
      symbol_ = EOF;
      return;
    }

    symbol_ = source_.stream.get();

    buffer_.push_back(symbol_);
  }

  auto PreviousLineEnd() -> const char* {
    return lines_.empty() ? buffer_.begin().base() : lines_.back().end();
  }

 private:
  lex::InputFile source_;

  std::vector<std::string_view> lines_;

  std::string buffer_;

  Location location_;

  char symbol_;
};

//////////////////////////////////////////////////////////////////////

}  // namespace lex
