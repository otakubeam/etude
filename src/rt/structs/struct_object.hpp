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

  StructDeclStatement* struct_decl;

  SBObject* data = nullptr;
};

// TODO: calculate offsets for filed access

}  // namespace rt

//////////////////////////////////////////////////////////////////////
