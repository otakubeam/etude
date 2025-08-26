#pragma once

#include <fmt/core.h>

#include <string>
#include <string_view>

#include <ast/error_at_location.hpp>

namespace parse::errors {

using ParseError = ErrorAtLocation;

template <const char * FormatStr>
struct LocationFormattedError : ParseError {
  LocationFormattedError(lex::Location location)
    : ParseError(
      std::move(location),
      fmt::format(FormatStr, location.Format()))
  {}
};

constexpr const char ParsePrimaryErrorFmt[]   = "Could not match primary expression at location {}";
using ParsePrimaryError   = LocationFormattedError<ParsePrimaryErrorFmt>;

constexpr const char ParseTrueBlockErrorFmt[] = "Could not parse true block at location {}";
using ParseTrueBlockError = LocationFormattedError<ParseTrueBlockErrorFmt>;

constexpr const char ParseNonLvalueErrorFmt[] = "Expected lvalue at location {}";
using ParseNonLvalueError = LocationFormattedError<ParseNonLvalueErrorFmt>;

constexpr const char ParseTypeErrorFmt[]      = "Could not parse the type at location {}";
using ParseTypeError      = LocationFormattedError<ParseTypeErrorFmt>;

struct ParseTokenError : ParseError {
  ParseTokenError(const std::string& tok, lex::Location location)
    : ParseError(
      std::move(location),
      fmt::format("Expected token {} at location {}", tok, location.Format()))
  {}
};

}  // namespace parse::errors
