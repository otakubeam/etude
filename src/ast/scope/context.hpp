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

  void InsertSymbol(Symbol symbol);
};

struct Context {
  ScopeLayer bindings{};

  std::string_view name;

  lex::Location location;

  size_t level = 0;  // For debug

  Context* parent = nullptr;

  std::vector<Context*> children{};

  void Print();

  Context* Find(std::string_view name);

  Context* MakeNewScopeLayer(lex::Location loc, std::string_view name);
};

using Type = types::Type;
using TypeTag = types::TypeTag;

Type* ConstructType(Type* ty, Context* ctx);
Type* ConstructTypeRec(Type* ty, Context* ctx);

}  // namespace ast::scope
