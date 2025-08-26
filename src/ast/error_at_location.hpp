#pragma once

#include <exception>
#include <string>

#include "lex/location.hpp"

// TODO: could store a bunch of locations.
//   It allows better compiler output if
//   error is very deep inside of imported modules.
//   Like in java
//     at abc.et, 0:1
//     at abd.et, 0:1
//     at abe.et, 10:14
// Java, as far as I remember, uses at file:line:col, but
//   I find it hard to read. So let's avoid double colon
//   sign and leave it just for line:col combo, which
//   is how it's used most of the time. 
class ErrorAtLocation : public std::exception {
public:
    ErrorAtLocation(lex::Location location, std::string description)
      : loc_(std::move(location)), desc_(std::move(description)) {}

    ErrorAtLocation(ErrorAtLocation&&) = default;
    ErrorAtLocation(const ErrorAtLocation&) = delete;

    virtual ~ErrorAtLocation() = default;

    const char* what() const noexcept override {
        return desc_.c_str();
    }

    lex::Location where() const noexcept {
        return loc_;
    }

private:
    lex::Location loc_;
    std::string desc_;
};
