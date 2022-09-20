#pragma once

#include <fmt/core.h>

#include <string>

namespace parse::errors {

struct ParseError : std::exception {
  std::string message;

  const char* what() const noexcept override {
    return message.c_str();
  }
};

struct ParsePrimaryError : ParseError {
  ParsePrimaryError(const std::string& location) {
    message = fmt::format("Could not match primary expression at location {}\n",
                          location);
  }
};

struct ParseTrueBlockError : ParseError {
  ParseTrueBlockError(const std::string& location) {
    message =
        fmt::format("Could not parse true block at location {}\n", location);
  }
};

struct ParseNonLvalueError : ParseError {
  ParseNonLvalueError(const std::string& location) {
    message = fmt::format("Expected lvalue at location {}\n", location);
  }
};

struct ParseTypeError : ParseError {
  ParseTypeError(const std::string& location) {
    message =
        fmt::format("Could not parse the type at location {}\n", location);
  }
};

struct ParseTokenError : ParseError {
  ParseTokenError(const std::string& tok, const std::string& location) {
    message = fmt::format("Expected token {} at location {}\n", tok, location);
  }
};

}  // namespace parse::errors
