#include <rt/primitive_type.hpp>

//////////////////////////////////////////////////////////////////////

PrimitiveType BinaryOp(char op_type, PrimitiveType lhs,
                              PrimitiveType rhs) {
  auto int_one = std::get<int>(lhs);
  auto int_two = std::get<int>(rhs);
  return {op_type == '+' ? int_one + int_two : int_one - int_two};
}

PrimitiveType Plus(PrimitiveType one, PrimitiveType two) {
  return BinaryOp('+', one, two);
}

PrimitiveType Minus(PrimitiveType one, PrimitiveType two) {
  return BinaryOp('-', one, two);
}

//////////////////////////////////////////////////////////////////////

PrimitiveType Bang(PrimitiveType one) {
  auto int_one = std::get<bool>(one);
  return {!int_one};
}

PrimitiveType Negate(PrimitiveType one) {
  auto int_one = std::get<int>(one);
  return {-int_one};
}

//////////////////////////////////////////////////////////////////////

std::string Format(PrimitiveType value) {
  return std::visit(
      [](const auto& x) {
        return fmt::format("{}", x);
      },
      value);
}

//////////////////////////////////////////////////////////////////////
