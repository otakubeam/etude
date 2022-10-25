#pragma once

#include <types/trait.hpp>

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
  TY_PARAMETER,
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
  Member* active;  // How do I know who is active?
  Member first;
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

inline Type builtin_int;
inline Type builtin_bool;
inline Type builtin_char;
inline Type builtin_unit;

inline Type* FindLeader(Type* a) {
  auto leader = a;
  while (a->leader) {
    leader = a->leader;
  }
  return leader;
}

inline void Unify(Type* a, Type* b);

inline void UnifyUnderlyingTypes(Type* a, Type* b) {
  // assert(la->tag == lb->tag);
  switch (a->tag) {
    case TypeTag::TY_PTR:
      Unify(a->as_ptr.underlying, b->as_ptr.underlying);
      break;

    case TypeTag::TY_ALIAS:
      if (a->as_alias.name != b->as_alias.name) {
        throw;
      }
      Unify(a->as_alias.underlying, b->as_alias.underlying);
      break;

    case TypeTag::TY_STRUCT: {
      if (a->as_struct.first.size() != b->as_struct.first.size()) {
        throw "";
      }

      for (size_t i = 0; i < a->as_struct.first.size(); i++) {
        // TODO:
      }

      std::abort();
      break;
    }

    case TypeTag::TY_FUN:
      // Unify(a->as_fun.param_pack, b->as_fun.param_pack);
      // Unify(a->as_fun.result_type, b->as_fun.result_type);

      std::abort();
      break;

    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_UNION:
      std::abort();

    default:
      break;
  }
}

inline void Unify(Type* a, Type* b) {
  auto la = FindLeader(a);
  auto lb = FindLeader(b);

  // Always make the la be be a variable
  if (lb->tag == TypeTag::TY_VARIABLE) {
    std::swap(la, lb);
  }

  if (la->tag == TypeTag::TY_VARIABLE) {
    la->leader = lb;
    // la->as_variable.constraints + lb->as_variable.constraints
  }

  if (la->tag == lb->tag) {
    UnifyUnderlyingTypes(la, lb);
    return;
  }

  throw "error";  // Error!
}

//////////////////////////////////////////////////////////////////////
};  // namespace types
