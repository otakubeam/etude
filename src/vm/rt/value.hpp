#pragma once

#include <vm/rt/primitive.hpp>

namespace vm::rt {

//////////////////////////////////////////////////////////////////////

struct StructObject;

using VmValue = std::variant<  //
    PrimitiveValue,            //
    StructObject*              //
    >;

}  // namespace vm::rt
