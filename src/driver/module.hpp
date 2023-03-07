#pragma once

#include <types/constraints/generate/algorithm_w.hpp>
#include <types/constraints/expand/expand.hpp>
#include <types/instantiate/instantiator.hpp>

#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>

#include <ast/declarations.hpp>

#include <qbe/ir_emitter.hpp>

//////////////////////////////////////////////////////////////////////

class CompilationDriver;

class Module {
 public:
  friend class Parser;

  void SetName(std::string_view name);

  void ExportTraitMethods();
  void BuildContext(CompilationDriver* driver);

  void MarkIntrinsics();
  void InferTypes(types::constraints::ConstraintSolver& solver);

  auto CompileMain(Declaration* main);
  auto CompileTests();
  void Compile(Declaration* main);
  std::string_view GetName() const;

  ast::scope::Symbol* GetExportedSymbol(std::string_view name);

 public:
  std::vector<std::string_view> imports_;
  std::vector<std::string_view> exported_;

 private:
  std::string_view name_;

  ast::scope::Context global_context;

  // Actual code item from this module
  std::vector<Declaration*> items_;

  // Functions that are marked @test
  std::vector<FunDeclaration*> tests_;
};

//////////////////////////////////////////////////////////////////////
