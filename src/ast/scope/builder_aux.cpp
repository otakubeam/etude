#include <ast/scope/context_builder.hpp>

#include <ast/declarations.hpp>
#include <ast/scope/symbol.hpp>

#include <utility>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

auto ContextBuilder::TryEnterModuleCtx(std::string_view name) -> Context* {
  if (auto symbol = current_context_->RetrieveSymbol(name)) {
    if (symbol->sym_type == SymbolType::MODULE) {
      return symbol->as_module->module_context_;
    }
  }

  return nullptr;
}

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

auto ContextBuilder::WithItemsSeparated(ModuleDeclaration* node) -> M* {
  auto module = new ModuleSymbol;

  // The following method separates different kinds of declarations

  ////////////////////////////////////////////////////////////////////

  auto Separate = [&module](Declaration* item) {
    if (auto function = item->as<FunDeclaration>()) {
      auto func_sym = new FunSymbol{.definition = function};
      func_sym->next = std::exchange(module->functions, func_sym);

      return;
    }

    if (auto trait = item->as<TraitDeclaration>()) {
      auto trait_sym = new TraitSymbol{.me = trait};
      trait_sym->next = std::exchange(module->traits, trait_sym);

      return;
    }

    if (auto impl = item->as<ImplDeclaration>()) {
      impl->next_ = std::exchange(module->impls, impl);

      return;
    }

    if (auto type = item->as<TypeDeclaration>()) {
      auto type_sym = new TypeSymbol{.definition = type};
      type_sym->next = std::exchange(module->types, type_sym);

      return;
    }
  };

  ////////////////////////////////////////////////////////////////////

  for (auto* item : node->exported_) {
    Separate(item);
  }

  for (auto* item : node->local_) {
    Separate(item);
  }

  return module;
}

//////////////////////////////////////////////////////////////////////

}  // namespace ast::scope
