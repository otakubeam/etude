#pragma once

#include <lex/location.hpp>

#include <string_view>

//////////////////////////////////////////////////////////////////////
// Forward

class FunDeclaration;
class TypeDeclaration;
class TraitDeclaration;
class ImplDeclaration;
struct Attribute;

namespace types {
struct Type;
}

//////////////////////////////////////////////////////////////////////

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

enum class SymbolType {
  GENERIC,  // <<<----- used in expand.hpp

  TRAIT_METHOD,
  MODULE,
  TRAIT,

  STATIC,
  CONST,
  TYPE,
  FUN,
  VAR,
};

//////////////////////////////////////////////////////////////////////

struct TypeSymbol {
  // Encodes the 'kind' of a type constructor
  types::Type* cons = nullptr;

  // The definition of the type
  TypeDeclaration* definition = nullptr;

  // ... Continuation of the types list
  TypeSymbol* next = nullptr;
};

struct BindingSymbol {
  // Inferred type for the binding
  types::Type* type = nullptr;
};

struct FunSymbol {
  // Attributes on the function
  // Possible values: @nomangle, @test
  Attribute* attrs = nullptr;

  // Inferred type for the method
  types::Type* type = nullptr;

  // The definition of method
  FunDeclaration* definition = nullptr;

  // ... Continuation of the functions list
  FunSymbol* next = nullptr;
};

//////////////////////////////////////////////////////////////////////

//
// The 'original' version in the trait declaration
//
struct TraitMethod {
  // Generic type for the trait method
  types::Type* type = nullptr;

  // The original trait declaration
  TraitDeclaration* trait = nullptr;

  // Possible 'blanket implementation' for a method
  FunDeclaration* blanket = nullptr;

  // ... Continuation of the trait methods list
  TraitMethod* next = nullptr;
};

//////////////////////////////////////////////////////////////////////

struct ImplMethod {
  // Specific type for this method
  types::Type* type = nullptr;

  // The definition of method
  FunDeclaration* definition = nullptr;

  // ... Continuation of the trait methods list
  ImplMethod* next = nullptr;
};

//////////////////////////////////////////////////////////////////////

//
// A symbol contatining all the item in its declaration sorted
// as well as the node in a list of all impls of this trait
//
// E.g: impl Show for Int { ... }
//
struct ImplSymbol {
  // Possible values: @overlap
  Attribute* attrs = nullptr;

  // The declaration for this impl
  ImplDeclaration* me = nullptr;

  // The list of all implemented methods
  ImplMethod* methods = nullptr;

  // The list of all defined associated types
  TypeSymbol* types = nullptr;

  // ... Continuation of the list of impls for a trait
  ImplSymbol* next = nullptr;
};

//////////////////////////////////////////////////////////////////////

struct TraitSymbol {
  // Possible values: none (yet)
  Attribute* attrs = nullptr;

  // The original trait declaration
  TraitDeclaration* me = nullptr;

  // List of all trait methods
  TraitMethod* methods = nullptr;

  // List of all associated types
  TypeSymbol* types = nullptr;

  // List of impls for the trait
  ImplSymbol* impls = nullptr;

  // ... the list of Traits in a module
  TraitSymbol* next = nullptr;
};

////////////////////////////////////////////////////////////////////////////////

struct ModuleSymbol {
  // The list of functions for the module
  FunSymbol* functions = nullptr;

  // The list of types for the module
  TypeSymbol* types = nullptr;

  // The list of traits for the module
  TraitSymbol* traits = nullptr;

  // The list of impls for the module
  ImplSymbol* impls = nullptr;

  // The list of functions marked @test for the module
  FunSymbol* tests = nullptr;
};

////////////////////////////////////////////////////////////////////////////////

struct Symbol {
  SymbolType sym_type;

  std::string_view name;

  union {
    BindingSymbol as_bind;   // var a; const b;
    ModuleSymbol as_module;  // vec; parse; sys;
    TraitMethod* as_method;  // show(...)
    TraitSymbol as_trait;    // trait Show { ... }
    TypeSymbol as_type;      // type Vec a = struct { ... }
    FunSymbol as_fun{};      // fun push vec item = ... ;
  };

  lex::Location declared_at;

  std::string_view Name() {
    return name;
  }

  FunDeclaration* GetFunctionDefinition() {
    FMT_ASSERT(sym_type == SymbolType::FUN, "Not a function symbol");
    return as_fun.definition;
  }

  types::Type* GetType();
};

//////////////////////////////////////////////////////////////////////

using Name = std::string_view;

Symbol MakeFunSymbol(FunDeclaration* node);
Symbol MakeVarSymbol(Name, types::Type*, lex::Location);
Symbol MakeTySymbol(Name, types::Type*, lex::Location);
Symbol MakeTraitSymbol(Name, TraitSymbol, lex::Location);
Symbol MakeTraitMethodSymbol(TraitMethod* method);

//////////////////////////////////////////////////////////////////////

}  // namespace ast::scope
