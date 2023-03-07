#include <ast/declarations.hpp>
#include <ast/scope/symbol.hpp>

#include <types/type.hpp>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

types::Type* Symbol::GetType() {
  switch (sym_type) {
    case SymbolType::VAR:
      return types::FindLeader(as_bind.type);
    case SymbolType::FUN:
      return types::FindLeader(as_fun.type);
    case SymbolType::TRAIT_METHOD:
      return types::FindLeader(as_method->blanket->type_);
    case SymbolType::TYPE:
    case SymbolType::GENERIC:
      return types::FindLeader(as_type.cons);
    case SymbolType::STATIC:
    case SymbolType::TRAIT:
    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

Symbol MakeFunSymbol(FunDeclaration* node) {
  return Symbol{.sym_type = SymbolType::FUN,
                .name = node->GetName(),
                .as_fun = {.definition = node},
                .declared_at = node->GetLocation()};
}

//////////////////////////////////////////////////////////////////////

Symbol MakeVarSymbol(std::string_view name, types::Type* ty,
                     lex::Location loc) {
  return Symbol{.sym_type = SymbolType::VAR,
                .name = name,
                .as_bind = {.type = ty},
                .declared_at = loc};
}

//////////////////////////////////////////////////////////////////////

Symbol MakeTySymbol(std::string_view name, types::Type* ty, lex::Location loc) {
  return Symbol{.sym_type = SymbolType::TYPE,
                .name = name,
                .as_type = {.cons = ty},
                .declared_at = loc};
}

//////////////////////////////////////////////////////////////////////

Symbol MakeTraitSymbol(std::string_view name, TraitSymbol trait,
                       lex::Location loc) {
  return Symbol{.sym_type = SymbolType::TRAIT,
                .name = name,
                .as_trait = trait,
                .declared_at = loc};
}

//////////////////////////////////////////////////////////////////////

Symbol MakeTraitMethodSym(TraitMethod* method) {
  return Symbol{.sym_type = SymbolType::TRAIT_METHOD,
                .name = method->blanket->GetName(),
                .as_method = method,
                .declared_at = method->blanket->GetLocation()};
}

//////////////////////////////////////////////////////////////////////

}  // namespace ast::scope
