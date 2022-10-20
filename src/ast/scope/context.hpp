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
  ScopeLayer type_tags;
  ScopeLayer functions;
  ScopeLayer bindings;

  Context* parent = nullptr;

  Context* MakeNewScopeLayer() {
    return new Context{
        .parent = this,
    };
  }
};

}  // namespace ast::scope
