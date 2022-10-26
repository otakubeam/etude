#pragma once

#include <ast/scope/symbol.hpp>

#include <unordered_map>
#include <deque>

namespace ast::scope {

struct ScopeLayer {
  using Storage = std::deque<Symbol>;
  using HashMap = std::unordered_map<std::string_view, Symbol*>;

  Storage symbols;
  HashMap symbol_map;

  void InsertSymbol(Symbol symbol) {
    symbols.push_back(std::move(symbol));
    symbol_map.insert({
        symbols.back().name,
        &symbols.back(),
    });
  }
};

struct Context {
  ScopeLayer type_tags{};
  ScopeLayer functions{};
  ScopeLayer bindings{};

  std::string_view name;

  lex::Location location;

  Context* parent = nullptr;

  Context* Find(std::string_view name) {
    if (bindings.symbol_map.contains(name)) {
      return this;
    }
    return parent == nullptr ? nullptr : parent->Find(name);
  }

  Context* MakeNewScopeLayer(lex::Location loc, std::string_view name) {
    return new Context{
        .name = name,
        .location = loc,
        .parent = this,
    };
  }

  void Print() {
    fmt::print(stderr, "[!] Popping context {} at {}\n", name,
               location.Format());

    fmt::print(stderr, "Bindings: \n");
    for (auto& sym : bindings.symbols) {
      fmt::print(stderr, "{}\n", sym.FormatSymbol());
    }

    fmt::print(stderr, "Functions: \n");
    for (auto& sym : functions.symbols) {
      fmt::print(stderr, "{}\n", sym.FormatSymbol());
    }

    fmt::print(stderr, "Type tags: \n");
    for (auto& sym : type_tags.symbols) {
      fmt::print(stderr, "{}\n", sym.FormatSymbol());
    }

    fmt::print(stderr, "\n");
  }
};

}  // namespace ast::scope
