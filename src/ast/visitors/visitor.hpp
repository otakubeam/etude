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

  virtual void VisitTypeDecl(TypeDeclaration* node) = 0;

  virtual void VisitVarDecl(VarDeclaration* node) = 0;

  virtual void VisitFunDecl(FunDeclaration* node) = 0;

  virtual void VisitTraitDecl(TraitDeclaration* node) = 0;

  virtual void VisitImplDecl(ImplDeclaration* node) = 0;

  // Patterns

  virtual void VisitBindingPat(BindingPattern* node) = 0;

  virtual void VisitDiscardingPat(DiscardingPattern* node) = 0;

  virtual void VisitLiteralPat(LiteralPattern* node) = 0;

  virtual void VisitStructPat(StructPattern* node) = 0;

  virtual void VisitVariantPat(VariantPattern* node) = 0;

  // Expressions

  virtual void VisitSeqExpr(SeqExpression* node) = 0;

  virtual void VisitAssign(AssignExpression* node) = 0;

  virtual void VisitLet(LetExpression* node) = 0;

  virtual void VisitComparison(ComparisonExpression* node) = 0;

  virtual void VisitBinary(BinaryExpression* node) = 0;

  virtual void VisitUnary(UnaryExpression* node) = 0;

  virtual void VisitDeref(DereferenceExpression* node) = 0;

  virtual void VisitAddressof(AddressofExpression* node) = 0;

  virtual void VisitIf(IfExpression* node) = 0;

  virtual void VisitMatch(MatchExpression* node) = 0;

  virtual void VisitNew(NewExpression* node) = 0;

  virtual void VisitBlock(BlockExpression* node) = 0;

  virtual void VisitFnCall(FnCallExpression* node) = 0;

  virtual void VisitIntrinsic(IntrinsicCall* node) = 0;

  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) = 0;

  virtual void VisitFieldAccess(FieldAccessExpression* node) = 0;

  virtual void VisitVarAccess(VarAccessExpression* node) = 0;

  virtual void VisitLiteral(LiteralExpression* node) = 0;

  virtual void VisitTypecast(TypecastExpression* node) = 0;

  virtual void VisitYield(YieldExpression* node) = 0;

  virtual void VisitReturn(ReturnExpression* node) = 0;

  virtual void VisitIndex(IndexExpression* node) = 0;
};

//////////////////////////////////////////////////////////////////////
