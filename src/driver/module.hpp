#pragma once

#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>
#include <types/instantiate/instantiator.hpp>
#include <types/check/algorithm_w.hpp>
#include <qbe/ir_emitter.hpp>

#include <ast/visitors/visitor.hpp>
#include <ast/declarations.hpp>

#include <lex/location.hpp>

//////////////////////////////////////////////////////////////////////

class CompilationDriver;

class Module {
 public:
  friend class Parser;

  void SetName(std::string_view name) {
    name_ = name;
  }

  void BuildContext(CompilationDriver* driver) {
    global_context.driver = driver;
    ast::scope::ContextBuilder ctx_builder{global_context};
    for (auto item : items_) item->Accept(&ctx_builder);
  }

  void MarkIntrinsics() {
    ast::elaboration::MarkIntrinsics mark;
    for (auto& r : items_) r = mark.Eval(r)->as<Declaration>();
  }

  void InferTypes() {
    types::check::AlgorithmW infer;
    for (auto r : items_) r->Accept(&infer);
  }

  void Compile(Declaration* main) {
    types::check::TemplateInstantiator inst(main);
    auto [funs, gen_ty_list] = inst.Flush();

    qbe::IrEmitter ir;
    ir.EmitTypes(std::move(gen_ty_list));

    for (auto f : funs) f->Accept(&ir);
  }

  std::string_view GetName() const {
    return name_;
  }

  std::vector<std::string_view> imports_;
  std::vector<std::string_view> exported_;

  ast::scope::Symbol* GetExportedSymbol(std::string_view name) {
    return global_context.FindLocalSymbol(name);
  }

  enum Type {
    Library,
    Executable,
  } mod_ty = Executable;

 private:
  std::string_view name_;

  ast::scope::Context global_context;

  // Actual code item from this module
  std::vector<Declaration*> items_;

  // Functions that are marked #[test]
  std::vector<Declaration*> tests_;
};

//////////////////////////////////////////////////////////////////////
