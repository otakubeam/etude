#pragma once

#include <fmt/core.h>
#include <lex/location.hpp>

#include <string_view>
#include <string>

namespace types {

struct Type;

enum class TraitTags {
  EQ,
  ORD,  // EQ a => ORD a

  ADD,  // only + : for pointers and numeric
  NUM,  // ADD a => NUM a

  CALLABLE,

  TYPES_EQ,
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

struct TypesEqual {
  Type* a{};
  Type* b{};
};

struct UserDefinedTrait {
  // Pointer to ast node or something
};

struct None {};

struct Trait {
  TraitTags tag;

  Type* bound = nullptr;

  union {
    None none;  // shut up [-Wmissing-field-initializers] warnings

    ConvertibleToTrait convertible_to;
    HasFieldTrait has_field;
    TypesEqual types_equal;
    UserDefinedTrait* user;
  };

  lex::Location location;
};

std::string FormatTrait(Trait& trait);

}  // namespace types
