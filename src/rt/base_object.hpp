#pragma once

#include <rt/primitive_object.hpp>
#include <rt/runtime_error.hpp>

namespace rt {

//////////////////////////////////////////////////////////////////////

struct IFunction;
struct StructObject;

using SBObject = std::variant<  //
    PrimitiveObject,            //
    IFunction*,                 //
    StructObject*               //
    >;

//////////////////////////////////////////////////////////////////////

template <typename T>
inline T GetPrim(const SBObject& object) {
  auto prim = std::get<PrimitiveObject>(object);
  return std::get<T>(prim);
}

template <typename T>
inline SBObject FromPrim(T value) {
  return SBObject{PrimitiveObject{value}};
}

/////////////////////////////////////////////////////////////////////

SBObject BinaryOp(char op_type, SBObject lhs, SBObject rhs);
SBObject Plus(SBObject one, SBObject two);
SBObject Minus(SBObject one, SBObject two);

/////////////////////////////////////////////////////////////////////

SBObject UnaryOp(char op_type, SBObject operand);
SBObject Bang(SBObject one);
SBObject Negate(SBObject one);

/////////////////////////////////////////////////////////////////////

std::string Format(const SBObject& object);
std::ostream& operator<<(std::ostream& os, const SBObject& object);

/////////////////////////////////////////////////////////////////////
}  // namespace rt
