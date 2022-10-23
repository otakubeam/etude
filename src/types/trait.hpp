#pragma once

#include <string_view>

namespace types {

enum class TraitTags {
  NUMERIC,
  CALLABLE,
  HAS_FIELD,
  IS_POINTER,

  USER_DEFINED,
};

struct UserDefinedTrait {
  // This is perhaphs stored in a symbol table?
  UserDefinedTrait* next;
};

struct Trait {
  TraitTags tag;

  union {
    std::string_view field_name;
    UserDefinedTrait* user = nullptr;
  };
};

}  // namespace types
