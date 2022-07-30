#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <variant>
#include <string>

//////////////////////////////////////////////////////////////////////

using PrimitiveType = std::variant<  //
    std::nullptr_t,                  //
    int,                             //
    bool,                            //
    std::string                      //
    >;

//////////////////////////////////////////////////////////////////////

PrimitiveType BinaryOp(char op_type, PrimitiveType lhs, PrimitiveType rhs);

PrimitiveType Plus(PrimitiveType one, PrimitiveType two);
PrimitiveType Minus(PrimitiveType one, PrimitiveType two);

PrimitiveType Bang(PrimitiveType one);
PrimitiveType Negate(PrimitiveType one);

std::string Format(PrimitiveType value);

//////////////////////////////////////////////////////////////////////
