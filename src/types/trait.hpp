#pragma once

#include <string_view>

namespace types {

struct Type;

enum class TraitTags {
  EQ,
  ORD,

  NUM,  // + and *

  DEREF,

  CALLABLE,
  HAS_FIELD,

  USER_DEFINED,
};

struct HasFieldTrait {
  std::string_view field_name;
  Type* field_type;
};

struct UserDefinedTrait {
  // Pointer to ast node or something
};

struct Trait {
  TraitTags tag;

  union {
    HasFieldTrait has_field;
    UserDefinedTrait* user = nullptr;
  };
};

}  // namespace types
