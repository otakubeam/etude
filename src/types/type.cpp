#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

Type::Arena type_store{};

//////////////////////////////////////////////////////////////////////

void CheckTypes() {
  auto& store = type_store;

  for (auto& t : store) {
    if (t.tag != TypeTag::TY_APP) {
      continue;
    }

    // Check that every TY_APP has a context attached

    if (!t.typing_context_) {
      fmt::print(stderr, "context:{:<20} \t\t\t type:{} \n",
                 (void*)t.typing_context_, FormatType(&t));
      std::abort();
    }

    // Also check that we can retreive the symbol from the symbol table

    if (!t.typing_context_->RetrieveSymbol(t.as_tyapp.name)) {
      fmt::print(stderr, "[!] Could not find type locally\n");

      t.typing_context_->Print();

      fmt::print(stderr, "context:{:<20} \t\t\t type:{} \n",
                 (void*)t.typing_context_, FormatType(&t));

      std::abort();
    }
  }
}

//////////////////////////////////////////////////////////////////////

void SetTyContext(types::Type* ty, ast::scope::Context* typing_context) {
  FMT_ASSERT(typing_context, "Not null");
  ty->typing_context_ = typing_context;

  switch (ty->tag) {
    case TypeTag::TY_PTR:
      SetTyContext(ty->as_ptr.underlying, typing_context);
      break;

    case TypeTag::TY_STRUCT:
      for (auto mem = ty->as_struct.members; mem; mem = mem->next) {
        SetTyContext(mem->ty, typing_context);
      }
      break;

    case TypeTag::TY_SUM:
      for (auto mem = ty->as_struct.members; mem; mem = mem->next) {
        if (mem->ty) {
          SetTyContext(mem->ty, typing_context);
        }
      }

      break;

    case TypeTag::TY_FUN:
      for (auto p = ty->as_fun.parameters; p; p = p->next) {
        SetTyContext(p->ty, typing_context);
      }

      SetTyContext(ty->as_fun.result_type, typing_context);
      break;

    case TypeTag::TY_APP:
      for (auto p = ty->as_tyapp.parameters; p; p = p->next) {
        SetTyContext(p->ty, typing_context);
      }
      break;

    case TypeTag::TY_UNION:
      std::abort();

    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_CONS:
    case TypeTag::TY_KIND:
    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////

}  // namespace types
