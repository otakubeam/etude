#pragma once

#include <vm/codegen/detail/function_symbol.hpp>
#include <vm/codegen/detail/struct_symbol.hpp>
#include <vm/codegen/frame_translator.hpp>

#include <vm/instr_translator.hpp>
#include <vm/elf_file.hpp>

#include <ast/visitors/template_visitor.hpp>
#include <ast/scope/environment.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <utility>

namespace vm::codegen {

class Compiler : public Visitor {
 public:
  Compiler();
  virtual ~Compiler();

  ElfFile Compile(TreeNode*);

  ////////////////////////////////////////////////////////////////////

  // virtual void VisitStatement(Statement* node) override;
  virtual void VisitVarDecl(VarDeclStatement* node) override;
  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* node) override;
  virtual void VisitStructDecl(StructDeclStatement* node) override;
  virtual void VisitReturn(ReturnStatement* node) override;
  virtual void VisitYield(YieldStatement* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  // virtual void VisitExpression(Expression*) override;
  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
  virtual void VisitNew(NewExpression* node) override;
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
  void EmitMemFetch(int16_t offset);

  int LookupVarAddress(std::string name);

  class AddrGenerator;

  auto GenAddress(Expression* expr, bool lvalue_context)
      -> std::vector<FatInstr>* {
    static std::vector<FatInstr> generation;

    if (auto lit = dynamic_cast<VarAccessExpression*>(expr)) {
      auto name = lit->GetName();
      int16_t offset = LookupVarAddress(name);
      fmt::print("{}\n", name);
      generation.push_back(FatInstr{
          .type = InstrType::PUSH_FP,
      });
      generation.push_back(FatInstr{
          .type = InstrType::ADD_ADDR,
          .offset = offset,
      });
    } else if (auto deref = dynamic_cast<DereferenceExpression*>(expr)) {
      deref->operand_->Accept(this);
      // TODO: do I need this?
      if (!lvalue_context) {
        generation.push_back(FatInstr{
            .type = InstrType::LOAD,
        });
      }
    } else if (auto field_access = dynamic_cast<FieldAccessExpression*>(expr)) {
      GenAddress(field_access->struct_expression_, true);
      generation.push_back(FatInstr{
          .type = InstrType::ADD_ADDR,
          .offset = GetFieldOffset(field_access),
      });
    }

    return &generation;
  }

 private:
  int16_t GetFieldOffset(FieldAccessExpression* field_access) {
    auto t = dynamic_cast<types::StructType*>(
        field_access->struct_expression_->GetType());

    auto struct_type_name = t->GetName();

    // Symbol encapsulate knowledge about the size of primitive fields
    // which might be differenct from implementation to implementation
    auto struct_symbol = structs_.Get(struct_type_name).value();

    auto offset = struct_symbol->SizeBefore(field_access->GetFieldName());

    return offset;
  }

  uint8_t GetValueSize(Expression* expr) {
    return GetTypeSize(expr->GetType());
  }

  uint8_t GetTypeSize(types::Type* type) {
    if (auto struct_type = dynamic_cast<types::StructType*>(type)) {
      return structs_.Get(struct_type->GetName()).value()->Size();
    }
    return 1;
  }

  void TranslateInstruction(FatInstr instr) {
    translator_->TranslateInstruction(instr);
  }

 private:
  std::optional<InstrTranslator> translator_;

  // Environment
  using StructEnv = Environment<detail::StructSymbol*>;
  StructEnv structs_ = StructEnv::MakeGlobal();

  // StackEmulation
  FrameTranslator* current_frame_ = nullptr;
};

}  // namespace vm::codegen
