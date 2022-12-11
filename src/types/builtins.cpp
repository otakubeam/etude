#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

Type builtin_int{.tag = TypeTag::TY_INT};
Type builtin_bool{.tag = TypeTag::TY_BOOL};
Type builtin_char{.tag = TypeTag::TY_CHAR};
Type builtin_unit{.tag = TypeTag::TY_UNIT};
Type builtin_never{.tag = TypeTag::TY_NEVER};

Type builtin_kind{.tag = TypeTag::TY_KIND};

//////////////////////////////////////////////////////////////////////

};  // namespace types
