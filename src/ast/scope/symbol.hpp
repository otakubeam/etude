#pragma once

#include <types/repr/struct_type.hpp>
#include <types/repr/fn_type.hpp>

#include <lex/location.hpp>

#include <string_view>
#include <vector>

namespace ast::scope {

enum class SymbolType {
  STATIC,
  TYPE,
  FUN,
  VAR,
};

struct StructSymbol {
  types::StructType* type;
};

struct FnSymbol {
  types::FnType* type = nullptr;
  bool type_is_known = false;

  // What form should this take?
  // std::vector<...> constraints;
};

struct VarbindSymbol {
  types::Type* type = nullptr;
  bool type_is_known = false;

  // What form should this take?
  // std::vector<...> constraints;
};

struct Symbol {
  SymbolType sym_type;

  // Fun, types and var can be incomplete
  // Can static be incomplete?
  bool is_complete = false;

  std::string_view name;

  union {
    FnSymbol as_fn_sym{};
    StructSymbol as_struct;
    VarbindSymbol as_varbind;
  };

  lex::Location declared_at;
  std::vector<lex::Location> uses{};

  std::string_view FormatSymbol() {
    return name;
  }
};

}  // namespace ast::scope
