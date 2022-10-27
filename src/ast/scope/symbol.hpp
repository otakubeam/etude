#pragma once

#include <types/type.hpp>

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
    StructSymbol as_struct;
    VarbindSymbol as_varbind;
  };

  lex::Location declared_at;
  std::vector<lex::Location> uses{};

  std::string_view FormatSymbol() {
    return name;
  }

  types::Type* GetType() {
    switch (sym_type) {
      case SymbolType::VAR:
        return types::FindLeader(as_varbind.type);
      case SymbolType::FUN:
        return types::FindLeader(as_fn_sym.type);
      case SymbolType::TYPE:
        return types::FindLeader(as_struct.type);
      case SymbolType::STATIC:
      default:
        std::abort();
    }
  }
};

}  // namespace ast::scope
