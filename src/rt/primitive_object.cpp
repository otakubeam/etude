#include <rt/primitive_object.hpp>

namespace rt {

//////////////////////////////////////////////////////////////////////

PrimitiveObject BinaryOp(char op_type, PrimitiveObject lhs, PrimitiveObject rhs) {
  auto int_one = std::get<int>(lhs);
  auto int_two = std::get<int>(rhs);
  return {op_type == '+' ? int_one + int_two : int_one - int_two};
}

PrimitiveObject Plus(PrimitiveObject one, PrimitiveObject two) {
  return BinaryOp('+', one, two);
}

PrimitiveObject Minus(PrimitiveObject one, PrimitiveObject two) {
  return BinaryOp('-', one, two);
}

//////////////////////////////////////////////////////////////////////

PrimitiveObject Bang(PrimitiveObject one) {
  auto int_one = std::get<bool>(one);
  return {!int_one};
}

PrimitiveObject Negate(PrimitiveObject one) {
  auto int_one = std::get<int>(one);
  return {-int_one};
}

//////////////////////////////////////////////////////////////////////

std::string Format(PrimitiveObject value) {
  return std::visit(
      [](const auto& x) {
        return fmt::format("{}", x);
      },
      value);
}

//////////////////////////////////////////////////////////////////////

PrimitiveObject FromSemInfo(lex::Token::SemInfo sem_info) {
  switch (sem_info.index()) {
      // std::monostate
    case 0:
      // unit
      return PrimitiveObject{nullptr};

      // std::string
    case 1:
      return PrimitiveObject{std::get<std::string>(sem_info)};

      // int
    case 2:
      return PrimitiveObject{std::get<int>(sem_info)};

    default:
      FMT_ASSERT(false, "\n Error: Non-exhaustive match \n");
  }
}

//////////////////////////////////////////////////////////////////////

}  // namespace rt
