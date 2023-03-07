#pragma once

#include <driver/module.hpp>

//////////////////////////////////////////////////////////////////////

void Module::SetName(std::string_view name) {
  name_ = name;
}

//////////////////////////////////////////////////////////////////////

void Module::ExportTraitMethods() {
  // Walk all items
  for (auto item : items_) {
    // Get traits
    if (auto trait = item->as<TraitDeclaration>()) {
      // Get their methods
      for (auto method : trait->assoc_items_) {
        // And export them
        exported_.push_back(method->GetName());
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

void Module::BuildContext(CompilationDriver* driver) {
  global_context.driver = driver;

  ast::scope::ContextBuilder ctx_builder{global_context};

  types::constraints::ExpandTypeVariables expand;

  for (auto item : items_) {
    item->Accept(&ctx_builder);
  }

  for (auto item : items_) {
    item->Accept(&expand);
  }
}

//////////////////////////////////////////////////////////////////////

void Module::MarkIntrinsics() {
  ast::elaboration::MarkIntrinsics mark;
  for (auto& r : items_) r = mark.Eval(r)->as<Declaration>();
}

//////////////////////////////////////////////////////////////////////

void Module::InferTypes(types::constraints::ConstraintSolver& solver) {
  solver.CollectAndSolve(items_);
}

//////////////////////////////////////////////////////////////////////

auto Module::CompileMain(Declaration* main) {
  types::instantiate::TemplateInstantiator inst(main);
  return inst.Flush();
}

//////////////////////////////////////////////////////////////////////

auto Module::CompileTests() {
  types::instantiate::TemplateInstantiator inst(tests_);
  return inst.Flush();
}

//////////////////////////////////////////////////////////////////////

void Module::Compile(Declaration* main) {
  auto [funs, gen_ty_list] = [&]() {
    return main ? CompileMain(main) : CompileTests();
  }();

  qbe::IrEmitter ir;
  ir.EmitTypes(std::move(gen_ty_list));

  for (auto f : funs) f->Accept(&ir);
}

//////////////////////////////////////////////////////////////////////

std::string_view Module::GetName() const {
  return name_;
}

//////////////////////////////////////////////////////////////////////

ast::scope::Symbol* Module::GetExportedSymbol(std::string_view name) {
  return global_context.FindLocalSymbol(name);
}

//////////////////////////////////////////////////////////////////////
