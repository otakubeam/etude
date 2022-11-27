#pragma once

#include <ast/visitors/visitor.hpp>

#include <lex/location.hpp>
#include <ast/statements.hpp>

//////////////////////////////////////////////////////////////////////

struct Module {
  enum Type {
    Library,
    Executable,
  } mod_ty;

  std::unordered_map<std::string_view, Module*> imports_;

  // For each import map `symbol -> module`
  std::unordered_map<std::string_view, Module*> module_of_;

  // Don't worry you just need their signatures, nothing more
  // Also might be a good idea to be able to `ask` module to
  // compile a concrete generic function for me

  // Maybe hashmap, as I will have to access this from other modules
  std::unordered_map<std::string_view, Statement*> exported_syms_;

  // Do I actually have any use for external symbols?
  // Probably no, it just means that I don't have to codegen them
  // [[deprecated]]
  // std::vector<Statement> external;

  // Actual code item from this module

  std::vector<Statement*> items_;

  // Functions that are marked #[test]

  std::vector<Statement*> tests_;
};

//////////////////////////////////////////////////////////////////////
