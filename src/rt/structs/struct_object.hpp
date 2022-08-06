#pragma once

#include <rt/base_object.hpp>

#include <ast/statements.hpp>

#include <cstdint>

//////////////////////////////////////////////////////////////////////

namespace rt {

struct StructObject {
  StructObject(StructDeclStatement* struct_decl) : struct_decl{struct_decl} {
    FMT_ASSERT(struct_decl, "Struct decl pointer is null");
    data = new SBObject[struct_decl->field_names_.size()];
  }

  SBObject* FieldAccess(std::string name) {
    size_t i = struct_decl->OffsetOf(name);
    return data + i;
  }

  StructDeclStatement* struct_decl;

  SBObject* data = nullptr;
};

}  // namespace rt

//////////////////////////////////////////////////////////////////////
