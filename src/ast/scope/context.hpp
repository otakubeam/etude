#pragma once

#include <ast/scope/symbol.hpp>

#include <unordered_map>
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

  Module& unit;
  lex::Location location;

  size_t level = 0;  // For debug

  Context* parent = nullptr;

  CompilationDriver* driver = nullptr;

  std::vector<Context*> children{};

  void Print();

  Context* FindLayer(std::string_view name);

  Symbol* RetrieveSymbol(std::string_view name, bool nothrow = false);

  Symbol* FindLocalSymbol(std::string_view name);
  Symbol* FindFromExported(std::string_view name, bool nothrow);

  // We need that after context builder done it's work. The problem
  //   is that AST node doesn't store the exact symbol in context,
  //   only context pointer. There may be shadowing, where we'll get
  //   the first occurence only.
  // What I invented to work around that is to get the first context
  //   where the name is defined. And then the last usage before the
  //   specified lex::Location. The clue is that the matched definition
  //   appears last with the name before the specified name usage. It
  //   has to be in the context of usage of in one of the parent context
  //   to be accessible. It's a hack, really, ast nodes for identifier
  //   usages like variable access or function call should know what
  //   AST node they refer to. And if AST nodes have global unique indexes,
  //   we can use such indexes in std::unordered_map inside an IR builder to
  //   store an llvm::Value* for example, that defines the value.
  Symbol* FindDeclForUsage(std::string_view name, const lex::Location& usage) {
    auto cmp_locations_le = [&](const lex::Location& lhs, const lex::Location& rhs) {
      return std::pair(lhs.lineno, lhs.columnno) <= std::pair(rhs.lineno, rhs.columnno);
    };

    if (bindings.symbol_map.contains(name)) {
      for (auto it = bindings.symbols.rbegin(); it != bindings.symbols.rend(); ++it) {
        if (it->name == name && cmp_locations_le(it->declared_at.position, usage)) {
          return &*it;
        }
      }
    }

    if (parent == nullptr) {
      return FindFromExported(name, true);
    }

    return parent->FindDeclForUsage(name, usage);
  }

  Symbol* RetrieveFromChild(std::string_view name);

  Context* MakeNewScopeLayer(lex::Location loc, std::string_view name);
};

}  // namespace ast::scope
