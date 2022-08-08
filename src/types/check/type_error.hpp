#pragma once

#include <fmt/core.h>

#include <string>

namespace types::check {

//////////////////////////////////////////////////////////////////////

struct TypeError {
  std::string msg;
};

//////////////////////////////////////////////////////////////////////

struct StructInitializationError : public TypeError {
  StructInitializationError() {
    msg = fmt::format("Bad struct construction");
  }
};

struct FieldAccessError : public TypeError {
  FieldAccessError(std::string field, std::string struct_var) {
    msg = fmt::format("No such field {} in struct {}", field, struct_var);
  }
};

//////////////////////////////////////////////////////////////////////

struct FnInvokeError : public TypeError {
  FnInvokeError(std::string fn_name, std::string /* loc_decl */,
                std::string loc_invoked) {
    msg = fmt::format(
        "Function {} at {} and its invocation at {} do not correspond",  //
        fn_name, "Unknown", loc_invoked);
  }
};

struct FnDeclarationError : public TypeError {
  FnDeclarationError() {
    // TODO: location of `return value`
    msg = fmt::format("Return type does not match the function declaration");
  }
};

struct FnBlockError : public TypeError {
  FnBlockError() {
    msg = fmt::format("Function block evaluates to different type");
  }
};

//////////////////////////////////////////////////////////////////////

struct IfError : public TypeError {};

struct IfCondError : public IfError {
  IfCondError(std::string /* location */) {
    msg = fmt::format("If expression at {} has non-bool condition",
                      "Unknown"  //
    );
  }
};

struct IfArmsError : public IfError {
  IfArmsError(std::string /* location */) {
    msg = fmt::format("If expression at {} has arms of different types",
                      "Unknown"  //
    );
  }
};

//////////////////////////////////////////////////////////////////////

struct ArithCmpError : public TypeError {
  ArithCmpError(std::string side) {
    msg = fmt::format("Comparison expression at {} has non-int {} operand",
                      "Unknown", side);
  }
};

struct ArithAddError : public TypeError {
  ArithAddError(std::string side) {
    msg = fmt::format("Binary expression at {} has non-int {} operand",
                      "Unknown", side);
  }
};

//////////////////////////////////////////////////////////////////////
};  // namespace types::check
