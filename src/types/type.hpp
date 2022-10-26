#pragma once

#include <types/trait.hpp>

#include <fmt/format.h>

#include <string_view>
#include <vector>
#include <string>

namespace types {

struct Type;

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
  TY_ALIAS,  // Idk how they are related to inference

  TY_VARIABLE,
  TY_PARAMETER,  // aplha, beta, etc... ?
};

struct Member {
  std::string_view field;
  Type* ty = nullptr;
};

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

struct Type {
  TypeTag tag = TypeTag::TY_VARIABLE;  // Unknown type

  Type* leader = nullptr;  // For use in union find

  // union {
  PtrType as_ptr{};
  FunType as_fun{};
  StructTy as_struct{};
  AliasType as_alias{};
  CoproductType as_copro{};
  TypeVariable as_variable{};
};

Type* FindLeader(Type* a);

std::string FormatType(Type& type);
std::string FormatStruct(Type& type);
std::string FormatFun(Type& type);

void Unify(Type* a, Type* b);
void UnifyUnderlyingTypes(Type* a, Type* b);

//////////////////////////////////////////////////////////////////////
};  // namespace types
