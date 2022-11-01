#include <ast/scope/symbol.hpp>
#include <types/type.hpp>

namespace ast::scope {

types::Type* Symbol::GetType() {
  switch (sym_type) {
    case SymbolType::VAR:
      return types::FindLeader(as_varbind.type);
    case SymbolType::FUN:
      return types::FindLeader(as_fn_sym.type);
    case SymbolType::TYPE:
      return types::FindLeader(as_type.type);
    case SymbolType::STATIC:
    default:
      std::abort();
  }
}

}  // namespace ast::scope
