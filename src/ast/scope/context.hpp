#pragma once

#include <ast/scope/symbol.hpp>

#include <unordered_map>
#include <vector>
#include <deque>

class CompilationDriver;

namespace ast::scope {

struct ScopeLayer {
  using Storage = std::deque<Symbol>;
  using HashMap = std::unordered_map<std::string_view, Symbol*>;

  Storage symbols;
  HashMap symbol_map;

  void InsertSymbol(Symbol symbol);
};

struct Context {
  ScopeLayer bindings{};

  std::string_view name;

  lex::Location location;

  size_t level = 0;  // For debug

  Context* parent = nullptr;

  CompilationDriver* driver = nullptr;

  std::vector<Context*> children{};

  void Print();

  void InsertSymbol(Symbol symbol);

  Context* FindLayer(std::string_view name);

  Symbol* RetrieveSymbol(std::string_view name);
  Symbol* FindLocalSymbol(std::string_view name);
  Symbol* RetrieveFromChild(std::string_view name);

  Context* MakeNewScopeLayer(lex::Location loc, std::string_view name);
};

}  // namespace ast::scope
