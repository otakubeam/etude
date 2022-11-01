#pragma once

#include <lex/location.hpp>

#include <string_view>
#include <vector>

namespace types {
struct Type;
}

namespace ast::scope {

enum class SymbolType {
  STATIC,
  TYPE,
  FUN,
  VAR,
};

struct StructSymbol {
  types::Type* type = nullptr;
};

struct FnSymbol {
  types::Type* type = nullptr;
};

struct VarbindSymbol {
  types::Type* type = nullptr;
};

struct Symbol {
  SymbolType sym_type;

  // Fun, types and var can be incomplete
  // Can static be incomplete?
  bool is_complete = false;

  std::string_view name;

  union {
    FnSymbol as_fn_sym{};
    StructSymbol as_type;
    VarbindSymbol as_varbind;
  };

  lex::Location declared_at;
  std::vector<lex::Location> uses{};

  std::string_view FormatSymbol() {
    return name;
  }

  types::Type* GetType();

  //////////////////////////////////////////////////////////////////////
};

}  // namespace ast::scope
