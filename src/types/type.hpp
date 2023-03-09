#pragma once

#include <types/constraints/trait.hpp>

#include <ast/scope/context.hpp>
#include <lex/token.hpp>

#include <fmt/format.h>

#include <unordered_map>
#include <string_view>
#include <vector>
#include <string>
#include <deque>

namespace types {

struct Type;

//////////////////////////////////////////////////////////////////////

enum class TypeTag {
  TY_INT,
  TY_BOOL,
  TY_CHAR,
  TY_UNIT,
  TY_NEVER,

  TY_BUILTIN,

  TY_PTR,
  TY_SUM,
  TY_UNION,
  TY_STRUCT,

  TY_FUN,

  TY_APP,   // Vec(Int), Maybe(String) -- concrete types
  TY_CONS,  // `Vec` in `type Vec t = { data: *t, ... }`;
  TY_KIND,  // *

  TY_VARIABLE,   // (Yet) Unknown type
  TY_PARAMETER,  // Generalized type parameters
};

//////////////////////////////////////////////////////////////////////

struct Member {
  Type* ty = nullptr;
  std::string_view field;

  // The continuation of the list of members
  Member* next = nullptr;
};

struct Parameter {
  Type* ty = nullptr;

  // The continuation
  Parameter* next = nullptr;
};

//////////////////////////////////////////////////////////////////////

struct TyVar {
  size_t id = 0;           // For easy identification
  Type* leader = nullptr;  // For use in union find
  Trait* constraints = nullptr;
};

struct PtrType {
  Type* underlying;
};

struct FunType {
  Parameter* parameters;
  Type* result_type;
};

struct StructTy {
  Member* members;
};

// Corresponds to the function symbol on the type level
struct TyConsType {
  std::string_view name;

  std::vector<lex::Token> param_pack{};

  Type* body = nullptr;

  // The signature of the symbol; corresponds to the
  // signature of a function
  size_t kind = 0;
};

// Corresponds to the function call on the type level
struct TyAppType {
  std::string_view name;
  Parameter* parameters;
};

//////////////////////////////////////////////////////////////////////

extern Type builtin_int;
extern Type builtin_bool;
extern Type builtin_char;
extern Type builtin_unit;
extern Type builtin_never;

extern Type builtin_kind;

//////////////////////////////////////////////////////////////////////

std::string FormatConstraints(Trait* constraints);
std::string FormatType(Type* type);
std::string Mangle(Type* type);

//////////////////////////////////////////////////////////////////////

struct Type {
  using Arena = std::deque<Type>;

  TypeTag tag = TypeTag::TY_VARIABLE;

  ast::scope::Context* typing_context_ = nullptr;

  union {
    TyVar as_var{};
    PtrType as_ptr;
    FunType as_fun;
    StructTy as_struct;
    TyAppType as_tyapp;
  };
  TyConsType as_tycons{};

  std::string Format() {
    return FormatType(this);
  }
};

//////////////////////////////////////////////////////////////////////

extern Type::Arena type_store;

//////////////////////////////////////////////////////////////////////

void CheckTypes();

Type* HintedOrNew(Type*);
Type* MakeTypeVar();
Type* MakeTypeVar(ast::scope::Context* ty_cons);

Type* MakeTypePtr(Type* under);
Type* MakeSumType(Member* fields);
Type* MakeStructType(Member* fields);
Type* MakeFunType(Parameter* parameters, Type* result_type);

Type* MakeTyApp(std::string_view name, Parameter* parameters);
Type* MakeTyCons(std::string_view name, std::vector<lex::Token> params,
                 Type* body, size_t kind, ast::scope::Context* context);

//////////////////////////////////////////////////////////////////////

void SetTyContext(types::Type* ty, ast::scope::Context* typing_context);

//////////////////////////////////////////////////////////////////////

Type* FindLeader(Type* ty);
Type* TypeStorage(Type* ty);
Type* ApplyTyconsLazy(Type* ty);

using Map = std::unordered_map<std::string_view, Type*>;
Type* SubstituteParameters(Type* subs, const Map& map);

using KnownParams = std::unordered_map<Type*, Type*>;
Type* InstituteParameters(Type* subs, const KnownParams& map);

//////////////////////////////////////////////////////////////////////

};  // namespace types
