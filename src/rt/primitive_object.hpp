#pragma once

#include <lex/token.hpp>

#include <fmt/format.h>

#include <cstdint>
#include <variant>
#include <string>

namespace rt {

//////////////////////////////////////////////////////////////////////

using PrimitiveObject = std::variant<  //
    std::nullptr_t,                  //
    int,                             //
    bool,                            //
    std::string                      //
    >;

//////////////////////////////////////////////////////////////////////

PrimitiveObject BinaryOp(char op_type, PrimitiveObject lhs, PrimitiveObject rhs);

PrimitiveObject Plus(PrimitiveObject one, PrimitiveObject two);
PrimitiveObject Minus(PrimitiveObject one, PrimitiveObject two);

PrimitiveObject Bang(PrimitiveObject one);
PrimitiveObject Negate(PrimitiveObject one);

std::string Format(PrimitiveObject value);
PrimitiveObject FromSemInfo(lex::Token::SemInfo sem_info);

//////////////////////////////////////////////////////////////////////

}  // namespace rt
