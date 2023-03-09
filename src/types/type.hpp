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

  TY_VARIABLE,
  TY_PARAMETER,  // Generalized type parameters
};

//////////////////////////////////////////////////////////////////////

struct Member {
  std::string_view field;
  Type* ty = nullptr;
};

//////////////////////////////////////////////////////////////////////

// This is also plain union type
struct StructTy {
  std::vector<Member> first;
};

struct SumType {
  std::vector<Member> first;
};

struct PtrType {
  Type* underlying;
};

struct FunType {
  std::vector<Type*> param_pack;
  Type* result_type;
};

// This is like a function symbol

struct TyConsType {
  std::string_view name;
  std::vector<lex::Token> param_pack{};

  Type* body = nullptr;
  Type* kind = nullptr;
};

// And this is like a function call

struct TyAppType {
  std::string_view name;
  std::vector<Type*> param_pack;
};

struct QualifiedGeneric {
  std::vector<Trait> constraints;
};

//////////////////////////////////////////////////////////////////////

extern Type builtin_int;
extern Type builtin_bool;
extern Type builtin_char;
extern Type builtin_unit;
extern Type builtin_never;

extern Type builtin_kind;

//////////////////////////////////////////////////////////////////////

std::string FormatType(Type& type);
std::string Mangle(Type& type);

//////////////////////////////////////////////////////////////////////

struct Type {
  using Arena = std::deque<Type>;

  Type* leader = nullptr;  // For use in union find

  size_t id = 0;  // For easy identification

  TypeTag tag = TypeTag::TY_VARIABLE;  // Unknown type

  ast::scope::Context* typing_context_ = nullptr;

  PtrType as_ptr{};
  FunType as_fun{};
  SumType as_sum{};
  StructTy as_struct{};
  TyAppType as_tyapp{};
  TyConsType as_tycons{};
  QualifiedGeneric as_parameter{};

  std::string Format() {
    return FormatType(*this);
  }
};

//////////////////////////////////////////////////////////////////////

void CheckTypes();

Type* HintedOrNew(Type*);
Type* MakeTypeVar();
Type* MakeTypeVar(ast::scope::Context* ty_cons);

Type* MakeTypePtr(Type* underlying);
Type* MakeFunType(std::vector<Type*> param_pack, Type* result_type);
Type* MakeTyApp(std::string_view name, std::vector<Type*> param_pack);
Type* MakeTyCons(std::string_view name, std::vector<lex::Token> params,
                 Type* body, Type* kind, ast::scope::Context* context);
Type* MakeStructType(std::vector<Member> fields);
Type* MakeSumType(std::vector<Member> fields);

auto MakeKindParamPack(size_t size) -> std::vector<Type*>;

//////////////////////////////////////////////////////////////////////

void SetTyContext(types::Type* ty, ast::scope::Context* typing_context);

//////////////////////////////////////////////////////////////////////

std::string FormatStruct(Type& type);
std::string FormatFun(Type& type);

//////////////////////////////////////////////////////////////////////

Type* FindLeader(Type* ty);
Type* TypeStorage(Type* ty);
Type* ApplyTyconsLazy(Type* ty);

using KnownParams = std::unordered_map<Type*, Type*>;
Type* Instantinate(Type* ty, KnownParams& map);

// a -> Vec(a) == b -> Vec(b)
// For use in testing, works on generalized types
bool TypesEquivalent(Type* lhs, Type* rhs,
                     std::unordered_map<size_t, size_t> map = {});

//////////////////////////////////////////////////////////////////////

};  // namespace types
