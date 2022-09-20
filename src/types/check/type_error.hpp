#pragma once

#include <lex/location.hpp>

#include <fmt/core.h>

#include <string>

namespace types::check {

//////////////////////////////////////////////////////////////////////

struct TypeError : std::exception {
  std::string message;

  const char* what() const noexcept override {
    return message.c_str();
  }
};

//////////////////////////////////////////////////////////////////////

struct StructInitializationError : public TypeError {
  StructInitializationError(lex::Location location) {
    message = fmt::format("Bad struct construction at location {}",
                          location.Format());
  }
};

struct FieldAccessError : public TypeError {
  FieldAccessError() = default;

  FieldAccessError(lex::Location location, std::string field,
                   std::string struct_var) {
    message = fmt::format("No such field '{}' in struct {} at {}", field,
                          struct_var, location.Format());
  }

  static FieldAccessError NotAStruct(lex::Location location,
                                     std::string var_name) {
    auto result = FieldAccessError{};
    result.message = fmt::format("Variable '{}' used at {} is not a struct",
                                 var_name, location.Format());
    return result;
  }
};

//////////////////////////////////////////////////////////////////////

struct FnInvokeError : public TypeError {
  FnInvokeError(std::string fn_name, lex::Location loc_invoked) {
    message =
        fmt::format("Function {} and its invocation at {} do not correspond",
                    fn_name, loc_invoked.Format());
  }
};

struct FnReturnStatementError : public TypeError {
  FnReturnStatementError(lex::Location location) {
    message =
        fmt::format("Return type at {} does not match the function declaration",
                    location.Format());
  }
};

struct FnBlockFinalError : public TypeError {
  FnBlockFinalError(lex::Location location) {
    message = fmt::format("Function block at {} evaluates to different type",
                          location.Format());
  }
};

//////////////////////////////////////////////////////////////////////

struct IfError : public TypeError {};

struct IfCondError : public IfError {
  IfCondError(lex::Location location) {
    message = fmt::format("If expression at {} has non-bool condition",
                          location.Format());
  }
};

struct IfArmsError : public IfError {
  IfArmsError(lex::Location location) {
    message = fmt::format("If expression at {} has arms of different types",
                          location.Format());
  }
};

//////////////////////////////////////////////////////////////////////

struct ArithCmpError : public TypeError {
  ArithCmpError(lex::Location location, std::string side) {
    message =
        fmt::format("Comparison expression at [{}] has non-int {} operand",
                    location.Format(), side);
  }
};

struct ArithAddError : public TypeError {
  ArithAddError(lex::Location location, std::string side) {
    message = fmt::format("Binary expression at [{}] has non-int {} operand",
                          location.Format(), side);
  }
};

struct ArithNegateError : public TypeError {
  ArithNegateError(lex::Location location) {
    message =
        fmt::format("Incorrect negation at location {}", location.Format());
  }
};

//////////////////////////////////////////////////////////////////////

struct DereferenceError : public TypeError {
  DereferenceError(lex::Location location) {
    message =
        fmt::format("Trying to dereference a non-pointer type at location {}",
                    location.Format());
  }
};

//////////////////////////////////////////////////////////////////////

struct AssignmentError : public TypeError {
  AssignmentError(lex::Location location) {
    message =
        fmt::format("Assignment at location {} has sides of different types",
                    location.Format());
  }
};

//////////////////////////////////////////////////////////////////////

struct VarAccessError : public TypeError {
  VarAccessError(lex::Location location) {
    message = fmt::format("Access of an undefined variable at location {}",
                          location.Format());
  }
};

};  // namespace types::check
