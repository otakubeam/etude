#pragma once

#include <string_view>

namespace types {

struct Type;

enum class TraitTags {
  EQ,
  ORD, // ORD a => EQ a

  ADD,  // only + : for pointers and numeric
  NUM,  // ADD a => NUM a

  CALLABLE,
  HAS_FIELD,
  CONVERTIBLE_TO,

  USER_DEFINED,
};

struct HasFieldTrait {
  std::string_view field_name{};
  Type* field_type = nullptr;
};

struct ConvertibleToTrait {
  Type* to_type{};
};

struct UserDefinedTrait {
  // Pointer to ast node or something
};

struct None {};

struct Trait {
  TraitTags tag;

  Type* bound;

  union {
    None none;  // shut up [-Wmissing-field-initializers] warnings

    ConvertibleToTrait convertible_to;
    HasFieldTrait has_field;
    UserDefinedTrait* user;
  };
};

}  // namespace types
