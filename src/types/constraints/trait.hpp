#pragma once

#include <lex/location.hpp>

#include <fmt/core.h>

#include <string_view>
#include <string>

namespace types {

struct Type;

enum class TraitTags {
  EQ,              //
  ORD,             // Eq a => Ord a
                   //
  ADD,             // only `+` : for pointers and numeric
  NUM,             // Add a => Num a
                   //
  INDEX,           // []
  CALLABLE,        // ()
                   //
  TYPES_EQ,        // ~
  HAS_FIELD,       // .name : type
  CONVERTIBLE_TO,  // ~>
                   //
  USER_DEFINED,    // Parsed, Show, etc..
};

struct HasFieldTrait {
  std::string_view field_name{};
  Type* field_type = nullptr;
};

struct ConvertibleToTrait {
  Type* to_type = nullptr;
};

struct TypesEqual {
  Type* a = nullptr;
  Type* b = nullptr;
};

struct UserDefinedTrait {
  // Pointer to ast node or something
};

struct None {};

struct Trait {
  TraitTags tag = TraitTags::TYPES_EQ;

  Type* bound = nullptr;

  union {
    None none;  // shut up [-Wmissing-field-initializers] warnings

    ConvertibleToTrait convertible_to;
    HasFieldTrait has_field;
    TypesEqual types_equal;
    UserDefinedTrait* user;
  };

  lex::Location location;

  Trait* next = nullptr;
};

Trait MakeTyEqTrait(Type* a, Type* b, lex::Location);
Trait MakeEqTrait(Type* bound, lex::Location loc);
Trait MakeOrdTrait(Type* bound, lex::Location loc);
Trait MakeHasFieldTrait(Type* bound, std::string_view name, Type* field_type,
                        lex::Location loc);

std::string FormatTrait(Trait& trait);
std::string FormatTraitNoType(Trait& trait);

}  // namespace types
