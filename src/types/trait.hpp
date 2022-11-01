#pragma once

#include <types/type.hpp>

#include <fmt/core.h>

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

  Type* bound;

  union {
    None none;  // shut up [-Wmissing-field-initializers] warnings

    ConvertibleToTrait convertible_to;
    HasFieldTrait has_field;
    TypesEqual types_equal;
    UserDefinedTrait* user;
  };
};

inline std::string FormatTrait(Trait& trait) {
  switch (trait.tag) {
    case TraitTags::EQ:
    case TraitTags::ORD:

    case TraitTags::ADD:
    case TraitTags::NUM:

    case TraitTags::CALLABLE:

    case TraitTags::TYPES_EQ:
      return fmt::format("{} ~ {}", FormatType(*trait.types_equal.a),
                         FormatType(*trait.types_equal.b));

    case TraitTags::HAS_FIELD:
      return fmt::format("{} .{} ~ {}", FormatType(*trait.bound),
                         trait.has_field.field_name,
                         FormatType(*trait.has_field.field_type));

    case TraitTags::CONVERTIBLE_TO:

    case TraitTags::USER_DEFINED:

    default:
      std::abort();
  }
}

}  // namespace types
