#pragma once

//////////////////////////////////////////////////////////////////////

class Expression;
class AssignExpression;
class ComparisonExpression;
class BinaryExpression;
class UnaryExpression;
class DereferenceExpression;
class AddressofExpression;
class FnCallExpression;
class IntrinsicCall;
class CompoundInitializerExpr;
class FieldAccessExpression;
class LiteralExpression;
class IfExpression;
class MatchExpression;
class NewExpression;
class BlockExpression;
class VarAccessExpression;
class TypecastExpression;
class ReturnExpression;
class YieldExpression;
class IndexExpression;
class SeqExpression;
class LetExpression;

//////////////////////////////////////////////////////////////////////

class TypeDeclaration;
class TraitDeclaration;
class ModuleDeclaration;
class ImplDeclaration;
class VarDeclaration;
class FunDeclaration;

//////////////////////////////////////////////////////////////////////

class BindingPattern;
class DiscardingPattern;
class LiteralPattern;
class StructPattern;
class VariantPattern;

//////////////////////////////////////////////////////////////////////

class Visitor {
 public:
  virtual ~Visitor() = default;

  // Declarations

  virtual void VisitTypeDecl(TypeDeclaration*) = 0;

  virtual void VisitVarDecl(VarDeclaration*) = 0;

  virtual void VisitFunDecl(FunDeclaration*) = 0;

  virtual void VisitTraitDecl(TraitDeclaration*) = 0;

  virtual void VisitModuleDecl(ModuleDeclaration*) = 0;

  virtual void VisitImplDecl(ImplDeclaration*) = 0;

  // Patterns

  virtual void VisitBindingPat(BindingPattern*) = 0;

  virtual void VisitDiscardingPat(DiscardingPattern*) = 0;

  virtual void VisitLiteralPat(LiteralPattern*) = 0;

  virtual void VisitStructPat(StructPattern*) = 0;

  virtual void VisitVariantPat(VariantPattern*) = 0;

  // Expressions

  virtual void VisitSeqExpr(SeqExpression*) = 0;

  virtual void VisitAssign(AssignExpression*) = 0;

  virtual void VisitLet(LetExpression*) = 0;

  virtual void VisitComparison(ComparisonExpression*) = 0;

  virtual void VisitBinary(BinaryExpression*) = 0;

  virtual void VisitUnary(UnaryExpression*) = 0;

  virtual void VisitDeref(DereferenceExpression*) = 0;

  virtual void VisitAddressof(AddressofExpression*) = 0;

  virtual void VisitIf(IfExpression*) = 0;

  virtual void VisitMatch(MatchExpression*) = 0;

  virtual void VisitNew(NewExpression*) = 0;

  virtual void VisitBlock(BlockExpression*) = 0;

  virtual void VisitFnCall(FnCallExpression*) = 0;

  virtual void VisitIntrinsic(IntrinsicCall*) = 0;

  virtual void VisitCompoundInitalizer(CompoundInitializerExpr*) = 0;

  virtual void VisitFieldAccess(FieldAccessExpression*) = 0;

  virtual void VisitVarAccess(VarAccessExpression*) = 0;

  virtual void VisitLiteral(LiteralExpression*) = 0;

  virtual void VisitTypecast(TypecastExpression*) = 0;

  virtual void VisitYield(YieldExpression*) = 0;

  virtual void VisitReturn(ReturnExpression*) = 0;

  virtual void VisitIndex(IndexExpression*) = 0;
};

//////////////////////////////////////////////////////////////////////
