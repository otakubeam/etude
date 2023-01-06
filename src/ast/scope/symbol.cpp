#include <ast/scope/symbol.hpp>
#include <types/type.hpp>

namespace ast::scope {

types::Type* Symbol::GetType() {
  switch (sym_type) {
    case SymbolType::VAR:
      return types::FindLeader(as_varbind.type);
    case SymbolType::FUN:
    case SymbolType::TRAIT_METHOD:
      return types::FindLeader(as_fn_sym.type);
    case SymbolType::TYPE:
    case SymbolType::GENERIC:
      return types::FindLeader(as_type.type);
    case SymbolType::STATIC:
    case SymbolType::TRAIT:
    default:
      std::abort();
  }
}

}  // namespace ast::scope
