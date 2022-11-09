#pragma once

#include <unordered_map>
#include <string_view>

namespace ast::elaboration {

enum class Intrinsic : uint8_t {
  PRINT,
  ASSERT,
  IS_NULL,
};

static std::unordered_map<std::string_view, Intrinsic> intrinsics_table{
    {"print", Intrinsic::PRINT},
    {"assert", Intrinsic::ASSERT},
    {"isNull", Intrinsic::IS_NULL},
};

}  // namespace ast::elaboration
