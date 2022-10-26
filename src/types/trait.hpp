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
  CONVERTIBLE_TO,

  USER_DEFINED,
};

struct HasFieldTrait {
  std::string_view field_name;
  Type* field_type;
};

struct ConvertibleToTrait {
  Type* to_type;
};

struct UserDefinedTrait {
  // Pointer to ast node or something
};

struct Trait {
  TraitTags tag;

  union {
    HasFieldTrait has_field;
    ConvertibleToTrait convertible_to;;
    UserDefinedTrait* user = nullptr;
  };
};

}  // namespace types
