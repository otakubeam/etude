#include <types/constraints/trait.hpp>
#include <types/type.hpp>

namespace types {

Trait MakeTyEqTrait(Type* a, Type* b, lex::Location loc) {
  return Trait{.tag = TraitTags::TYPES_EQ,
               .types_equal = {.a = a, .b = b},
               .location = loc};
}

Trait MakeEqTrait(Type* bound, lex::Location loc) {
  return Trait{
      .tag = TraitTags::EQ, .bound = bound, .none = {}, .location = loc};
}

Trait MakeOrdTrait(Type* bound, lex::Location loc) {
  return Trait{
      .tag = TraitTags::ORD, .bound = bound, .none = {}, .location = loc};
}

Trait MakeHasFieldTrait(Type* bound, std::string_view name, Type* field_type,
                        lex::Location loc) {
  return Trait{
      .tag = TraitTags::HAS_FIELD,
      .bound = bound,
      .has_field = {.field_name = name, .field_type = field_type},
      .location = loc,
  };
}

std::string FormatTrait(Trait& trait) {
  switch (trait.tag) {
    case TraitTags::EQ:
      return fmt::format("Eq {}", FormatType(*trait.bound));

    case TraitTags::ORD:
      return fmt::format("Ord {}", FormatType(*trait.bound));

    case TraitTags::ADD:
      return fmt::format("Add {}", FormatType(*trait.bound));

    case TraitTags::NUM:
      return fmt::format("Num {}", FormatType(*trait.bound));

    case TraitTags::CALLABLE:
      return fmt::format("Call {}", FormatType(*trait.bound));

    case TraitTags::TYPES_EQ:
      return fmt::format("{} ~ {}", FormatType(*trait.types_equal.a),
                         FormatType(*trait.types_equal.b));

    case TraitTags::HAS_FIELD:
      return fmt::format("{} .{} ~ {}", FormatType(*trait.bound),
                         trait.has_field.field_name,
                         FormatType(*trait.has_field.field_type));

    case TraitTags::CONVERTIBLE_TO:
      return fmt::format("{} ~> {}", FormatType(*trait.bound),
                         FormatType(*trait.convertible_to.to_type));

    case TraitTags::USER_DEFINED:
      return fmt::format("User");

    default:
      std::abort();
  }
}

}  // namespace types
