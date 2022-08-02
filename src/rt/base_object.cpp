#include <rt/base_object.hpp>

//////////////////////////////////////////////////////////////////////

SBObject BinaryOp(char op_type, SBObject lhs, SBObject rhs) {
  try {
    auto prim_one = std::get<PrimitiveType>(lhs);
    auto prim_two = std::get<PrimitiveType>(rhs);
    return {BinaryOp(op_type, prim_one, prim_two)};
  } catch (std::bad_variant_access&) {
    throw RuntimeError{};
  }
}

SBObject Plus(SBObject one, SBObject two) {
  return BinaryOp('+', one, two);
}

SBObject Minus(SBObject one, SBObject two) {
  return BinaryOp('-', one, two);
}

//////////////////////////////////////////////////////////////////////

SBObject UnaryOp(char op_type, SBObject operand) {
  try {
    PrimitiveType prim_operand = std::get<PrimitiveType>(operand);
    return {op_type == '!' ? Bang(prim_operand)  //
                           : Negate(prim_operand)};
  } catch (std::bad_variant_access&) {
    throw RuntimeError{};
  }
}

SBObject Bang(SBObject one) {
  return UnaryOp('!', one);
}

SBObject Negate(SBObject one) {
  return UnaryOp('-', one);
}

//////////////////////////////////////////////////////////////////////

std::string Format(const SBObject& object) {
  try {
    PrimitiveType prim_operand = std::get<PrimitiveType>(object);
    return Format(prim_operand);
  } catch (std::bad_variant_access&) {
    throw RuntimeError{};
  }
}

/////////////////////////////////////////////////////////////////////

// For catch2 outputting meaningful objects
std::ostream& operator<<(std::ostream& os,  //
                         const SBObject& object) {
  return os << Format(object);
}

/////////////////////////////////////////////////////////////////////
