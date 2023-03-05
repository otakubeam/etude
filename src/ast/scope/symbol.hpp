#pragma once

#include <lex/location.hpp>

#include <string_view>
#include <vector>

class FunDeclaration;
class TraitDeclaration;
class ImplDeclaration;
struct Attribute;

namespace types {
struct Type;
}

namespace ast::scope {

enum class SymbolType {
  GENERIC,  // <<<----- used in expand.hpp

  TRAIT_METHOD,
  TRAIT,

  STATIC,
  TYPE,
  FUN,
  VAR,
};

struct StructSymbol {
  types::Type* type = nullptr;
};

struct FnSymbol {
  size_t argnum = 0;
  types::Type* type = nullptr;
  FunDeclaration* def = nullptr;
  TraitDeclaration* trait = nullptr;
  Attribute* attrs = nullptr;
};

struct TraitSymbol {
  TraitDeclaration* decl;
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
    TraitSymbol as_trait;
    StructSymbol as_type;
    VarbindSymbol as_varbind;
  };

  lex::Location declared_at;
  std::vector<lex::Location> uses{};

  std::string_view FormatSymbol() {
    return name;
  }

  FunDeclaration* GetFunctionDefinition() {
    FMT_ASSERT(sym_type == SymbolType::FUN, "Not a function symbol");
    return as_fn_sym.def;
  }

  types::Type* GetType();

  //////////////////////////////////////////////////////////////////////
};

}  // namespace ast::scope
