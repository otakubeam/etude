#pragma once

#include <rt/user_defined_type.hpp>
#include <rt/primitive_type.hpp>
#include <rt/type_error.hpp>

//////////////////////////////////////////////////////////////////////

struct IFunction;

using SBObject = std::variant<  //
    PrimitiveType,              //
    IFunction*,                 //
    UserDefinedType*            //
    >;

//////////////////////////////////////////////////////////////////////

template <typename T>
inline T GetPrim(const SBObject& object) {
  auto prim = std::get<PrimitiveType>(object);
  return std::get<T>(prim);
}

template <typename T>
inline SBObject FromPrim(T value) {
  return SBObject{PrimitiveType{value}};
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
