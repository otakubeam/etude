#include <ast/scope/context.hpp>

namespace ast::scope {

void ScopeLayer::InsertSymbol(Symbol symbol) {
  symbols.push_back(std::move(symbol));
  symbol_map.insert({
      symbols.back().name,
      &symbols.back(),
  });
}

Context* Context::Find(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return this;
  }
  if (functions.symbol_map.contains(name)) {
    return this;
  }
  if (type_tags.symbol_map.contains(name)) {
    return this;
  }
  return parent == nullptr ? nullptr : parent->Find(name);
}

Context* Context::MakeNewScopeLayer(lex::Location loc, std::string_view name) {
  auto child = new Context{
      .name = name,
      .location = loc,
      .level = level + 1,
      .parent = this,
  };
  children.push_back(child);
  return child;
}

void Context::Print() {
  fmt::print(stderr, "[!] Context {} at {}, level {}\n", name,
             location.Format(), level);

  fmt::print(stderr, "Bindings: \n");
  for (auto& sym : bindings.symbols) {
    fmt::print(stderr, "{}:{}\n", sym.FormatSymbol(),
               types::FormatType(*sym.as_varbind.type));
  }

  fmt::print(stderr, "Functions: \n");
  for (auto& sym : functions.symbols) {
    fmt::print(stderr, "{}:{}\n", sym.FormatSymbol(),
               types::FormatType(*sym.as_fn_sym.type));
  }

  fmt::print(stderr, "Type tags: \n");
  for (auto& sym : type_tags.symbols) {
    fmt::print(stderr, "{}:{}\n", sym.FormatSymbol(),
               types::FormatType(*sym.as_struct.type));
  }

  fmt::print(stderr, "\n");

  for (auto& ch : children) {
    ch->Print();
  }
}

}  // namespace ast::scope
