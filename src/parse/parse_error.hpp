#pragma once

#include <ostream>

struct ParseError {
  const char* msg;
};

// For catch2 outputting meaningful objects
inline std::ostream& operator<<(std::ostream& os,  //
                                const ParseError& error) {
  return os << error.msg;
}
