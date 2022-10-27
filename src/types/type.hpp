#pragma once

#include <types/trait.hpp>

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

  TY_BUILTIN,

  TY_PTR,
  TY_UNION,
  TY_STRUCT,

  TY_FUN,
  TY_ALIAS,  // rename to ty_cons

  TY_VARIABLE,
  TY_PARAMETER,  // aplha, beta, etc... ?
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

struct CoproductType {
  std::vector<Member> first;
};

struct PtrType {
  Type* underlying;
};

struct FunType {
  std::vector<Type*> param_pack;
  Type* result_type;
};

struct AliasType {
  std::string_view name;
  Type* underlying = nullptr;
};

struct TypeVariable {
  Trait* constraints;
};

//////////////////////////////////////////////////////////////////////

extern Type builtin_int;
extern Type builtin_bool;
extern Type builtin_char;
extern Type builtin_unit;

//////////////////////////////////////////////////////////////////////

struct Type {
  using Arena = std::deque<Type>;
  inline static Arena type_store{};

  Type* leader = nullptr;  // For use in union find

  size_t id = 0;  // For easy identification

  TypeTag tag = TypeTag::TY_VARIABLE;  // Unknown type

  // union {
  PtrType as_ptr{};
  FunType as_fun{};
  StructTy as_struct{};
  AliasType as_alias{};
  CoproductType as_copro{};
  TypeVariable as_variable{};
};

//////////////////////////////////////////////////////////////////////

Type* HintedOrNew(Type*);
Type* MakeTypeVar();
Type* MakeTypePtr(Type* underlying);
Type* MakeFunType(std::vector<Type*> param_pack, Type* result_type);

//////////////////////////////////////////////////////////////////////

std::string FormatType(Type& type);
std::string FormatStruct(Type& type);
std::string FormatFun(Type& type);

//////////////////////////////////////////////////////////////////////

Type* FindLeader(Type* a);

void Unify(Type* a, Type* b);
void UnifyUnderlyingTypes(Type* a, Type* b);

using KnownParams = std::unordered_map<Type*, Type*>;
Type* Instantinate(Type* ty, KnownParams& map);

void Generalize(Type* ty);

//////////////////////////////////////////////////////////////////////

};  // namespace types
