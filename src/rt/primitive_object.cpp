#include <rt/primitive_object.hpp>

namespace rt {

//////////////////////////////////////////////////////////////////////

PrimitiveType BinaryOp(char op_type, PrimitiveType lhs, PrimitiveType rhs) {
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

PrimitiveType FromSemInfo(lex::Token::SemInfo sem_info) {
  switch (sem_info.index()) {
      // std::monostate
    case 0:
      // unit
      return PrimitiveType{nullptr};

      // std::string
    case 1:
      return PrimitiveType{std::get<std::string>(sem_info)};

      // bool
    case 2:
      FMT_ASSERT(false, "\n Error: Unreachable \n");

      // int
    case 3:
      return PrimitiveType{std::get<int>(sem_info)};

    default:
      FMT_ASSERT(false, "\n Error: Non-exhaustive match \n");
  }
}

//////////////////////////////////////////////////////////////////////

}  // namespace rt
