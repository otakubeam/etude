#include <driver/compil_driver.hpp>
#include <ast/scope/context.hpp>
#include <types/type.hpp>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

void ScopeLayer::InsertSymbol(Symbol symbol) {
  symbols.push_back(std::move(symbol));
  symbol_map.insert({
      symbols.back().name,
      &symbols.back(),
  });
}

//////////////////////////////////////////////////////////////////////

Context* Context::Find(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return this;
  }
  return parent == nullptr ? nullptr : parent->Find(name);
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::RetrieveSymbol(std::string_view name) {
  if (auto f = FindLocalSymbol(name)) {
    return f;
  }
  return FindFromExported(name);
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::FindLocalSymbol(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return bindings.symbol_map.at(name);
  }
  return parent == nullptr ? nullptr : parent->FindLocalSymbol(name);
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::FindFromExported(std::string_view name) {
  if (auto mod = driver->GetModuleOf(name)) {
    if (auto sym = mod->GetExportedSymbol(name)) {
      return sym;
    }
  }

  throw std::runtime_error{fmt::format("No such symbol {}\n", name)};
}

//////////////////////////////////////////////////////////////////////

Context* Context::MakeNewScopeLayer(lex::Location loc, std::string_view name) {
  auto child = new Context{
      .name = name,
      .location = loc,
      .level = level + 1,
      .parent = this,
      .driver = driver,
  };
  children.push_back(child);
  return child;
}

//////////////////////////////////////////////////////////////////////

void Context::Print() {
  fmt::print("[!] Context {} at {}, level {}\n", name, location.Format(),
             level);

  fmt::print("Bindings: \n");
  for (auto& sym : bindings.symbols) {
    fmt::print("{}:{}\n", sym.FormatSymbol(),
               types::FormatType(*sym.GetType()));
  }

  fmt::print("\n\n\n");

  for (auto& ch : children) {
    ch->Print();
  }
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::RetrieveFromChild(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return bindings.symbol_map.at(name);
  }

  for (auto& ch : children) {
    if (auto s = ch->RetrieveFromChild(name))
      return s;
  }

  return nullptr;
}

}  // namespace ast::scope
