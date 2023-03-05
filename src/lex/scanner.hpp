#pragma once

#include <lex/token_type.hpp>
#include <lex/location.hpp>

#include <fmt/core.h>

#include <string_view>
#include <iostream>
#include <sstream>
#include <istream>
#include <vector>
#include <span>

namespace lex {

//////////////////////////////////////////////////////////////////////

class Scanner {
 public:
  Scanner(std::string_view filename, std::istream& source) {
    location_.filename = filename;

    std::stringstream ss;
    ss << source.rdbuf();
    buffer_ = ss.str();
  }

  void MoveRight() {
    switch (CurrentSymbol()) {
      case '\n':

        CutLine();
        location_.lineno += 1;
        location_.columnno = 0;

        break;

      case EOF:
        break;

      default:
        location_.columnno += 1;
    }

    offset_ += 1;
  }

  template <auto F>
  std::string_view ViewWhile() {
    auto start_pos = buffer_.c_str() + offset_;

    // For example:
    //   1. while not " char
    //   2. while alphanumeric

    while (F(CurrentSymbol())) {
      MoveRight();
    }

    return std::string_view(start_pos, buffer_.c_str() + offset_);
  }

  void MoveNextLine() {
    while (CurrentSymbol() != '\n' && buffer_.size() != offset_) {
      MoveRight();
    }

    // Finally, move to the next line
    MoveRight();
  }

  void CutLine() {
    lines_.push_back(std::string_view{
        PreviousLineEnd(),  // starting from the last line
        std::string_view{buffer_}
            .substr(0, offset_)
            .end(),  // to the end of the already read buffer
    });
  }

  char CurrentSymbol() {
    return offset_ < buffer_.size() ? buffer_[offset_] : EOF;
  }

  char PeekNextSymbol() {
    auto next_offset = offset_ + 1;
    return next_offset < buffer_.size() ? buffer_[next_offset] : EOF;
  }

  // strtod, strtoll require buffer position
  const char* GetBufferPosition() const {
    return buffer_.c_str() + offset_;
  }

  Location GetLocation() const {
    return location_;
  }

  std::span<std::string_view> GetLines() {
    return lines_;
  }

 private:
  auto PreviousLineEnd() -> const char* {
    return lines_.empty() ? buffer_.begin().base() : lines_.back().end();
  }

 private:
  std::vector<std::string_view> lines_;

  std::size_t offset_ = 0;

  std::string buffer_;

  Location location_;
};

//////////////////////////////////////////////////////////////////////

}  // namespace lex
