#pragma once

#include <vm/codegen/detail/function_symbol.hpp>
#include <vm/codegen/detail/struct_symbol.hpp>
#include <vm/codegen/frame_translator.hpp>
#include <vm/chunk.hpp>

#include <types/check/type_checker.hpp>

#include <ast/visitors/template_visitor.hpp>
#include <ast/scope/environment.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

namespace vm::codegen {

class Compiler : public Visitor {
 public:
  Compiler();
  virtual ~Compiler();

  ExecutableChunk Compile(TreeNode*);
  static std::vector<ExecutableChunk>* CompileScript(TreeNode*);

  ////////////////////////////////////////////////////////////////////

  struct ReturnedValue {
    // TODO: implement
  };

  virtual void VisitStatement(Statement* node) override;
  virtual void VisitVarDecl(VarDeclStatement* node) override;
  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* node) override;
  virtual void VisitStructDecl(StructDeclStatement* node) override;
  virtual void VisitReturn(ReturnStatement*) override;
  virtual void VisitYield(YieldStatement*) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  virtual void VisitExpression(Expression*) override;
  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
  virtual void VisitBlock(BlockExpression* node) override;
  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression*) override;
  virtual void VisitFnCall(FnCallExpression* node) override;
  virtual void VisitStructConstruction(StructConstructionExpression*) override;
  virtual void VisitFieldAccess(FieldAccessExpression* node) override;
  virtual void VisitVarAccess(VarAccessExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* node) override;

 private:
  // Belongs to ExecutableChunk, move it!
  void AddIntegerConsnant(int value);

  int LookupVarAddress(std::string name);
  void MabyeEmitMemFetch(int offset);

 private:
  ExecutableChunk chunk_;

  bool emit_mem_fetch_ = true;

  std::vector<ExecutableChunk>* compiled_chunks_ = nullptr;

  // Environment
  using StructEnv = Environment<detail::StructSymbol*>;
  StructEnv structs_ = StructEnv::MakeGlobal();

  using FunctionsEnv = Environment<detail::FunctionSymbol>;
  FunctionsEnv functions_ = FunctionsEnv::MakeGlobal();

  // StackEmulation
  FrameTranslator* current_frame_ = nullptr;
};

}  // namespace vm::codegen
