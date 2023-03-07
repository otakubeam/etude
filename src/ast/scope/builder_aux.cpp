#include <ast/scope/context_builder.hpp>

#include <ast/declarations.hpp>

#include <utility>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

void ContextBuilder::EnterScopeLayer(lex::Location loc, std::string_view name) {
  current_context_ = current_context_->MakeNewScopeLayer(loc, name);
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::PopScopeLayer() {
  current_context_ = current_context_->parent;
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::MaybeEval(TreeNode* node) {
  if (node) {
    Eval(node);
  }
}

//////////////////////////////////////////////////////////////////////

auto ContextBuilder::WithItemsSeparated(TraitDeclaration* node) -> T {
  TraitSymbol trait;

  for (auto* item : node->assoc_items_) {
    // An associated method

    if (auto assoc_method = item->as<FunDeclaration>()) {
      auto method = new TraitMethod{.blanket = assoc_method};
      method->next = std::exchange(trait.methods, method);

      continue;
    }

    // An associated type

    if (auto associated_type = item->as<TypeDeclaration>()) {
      auto type = new TypeSymbol{.definition = associated_type};
      type->next = std::exchange(trait.types, type);

      continue;
    }
  }

  return trait;
}

//////////////////////////////////////////////////////////////////////

auto ContextBuilder::WithItemsSeparated(ImplDeclaration* node) -> I* {
  auto impl = new ImplSymbol{.me = node};

  for (auto* item : node->assoc_items_) {
    // An associated method

    if (auto associated_method = item->as<FunDeclaration>()) {
      auto method = new ImplMethod{.definition = associated_method};
      method->next = std::exchange(impl->methods, method);

      continue;
    }

    // An associated type

    if (auto associated_type = item->as<TypeDeclaration>()) {
      auto type = new TypeSymbol{.definition = associated_type};
      type->next = std::exchange(impl->types, type);

      continue;
    }
  }

  return impl;
}

//////////////////////////////////////////////////////////////////////

}  // namespace ast::scope
