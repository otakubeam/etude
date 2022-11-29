#pragma once

#include <vm/codegen/detail/measure.hpp>
#include <vm/codegen/frame_translator.hpp>

#include <vm/instr_translator.hpp>
#include <vm/elf_file.hpp>

#include <ast/declarations.hpp>

#include <utility>

namespace vm::codegen {

class Compiler : public Visitor {
 public:
  Compiler();
  virtual ~Compiler();

  ElfFile Compile(TreeNode*);

  ////////////////////////////////////////////////////////////////////

  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitReturn(ReturnStatement* node) override;
  virtual void VisitYield(YieldStatement* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  ////////////////////////////////////////////////////////////////////

  virtual void VisitVarDecl(VarDeclStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* node) override;
  virtual void VisitTypeDecl(TypeDeclStatement* node) override;
  virtual void VisitTraitDecl(TraitDeclaration* node) override;

  ////////////////////////////////////////////////////////////////////

  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
  virtual void VisitNew(NewExpression* node) override;
  virtual void VisitBlock(BlockExpression* node) override;
  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression*) override;
  virtual void VisitFnCall(FnCallExpression* node) override;
  virtual void VisitIntrinsic(IntrinsicCall* node) override;
  virtual void VisitFieldAccess(FieldAccessExpression* node) override;
  virtual void VisitVarAccess(VarAccessExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* node) override;
  virtual void VisitTypecast(TypecastExpression* node) override;
  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;

 private:
  void EmitMemFetch(int16_t offset);
  void TranslateInstruction(FatInstr instr);

  auto GenAddress(Expression* expr) -> std::vector<FatInstr>*;
  int LookupVarAddress(std::string_view name);

  int16_t GetFieldOffset(FieldAccessExpression* fa);
  int16_t GetFieldOffset(types::Type* t, std::string_view field);

  uint8_t GetValueSize(Expression* exp);
  uint8_t GetTypeSize(types::Type* t);

 private:
  detail::SizeMeasure measure_;

  std::optional<InstrTranslator> translator_;

  // StackEmulation
  FrameTranslator* current_frame_ = nullptr;
};

}  // namespace vm::codegen
