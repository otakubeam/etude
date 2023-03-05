#pragma once

//////////////////////////////////////////////////////////////////////

class Expression;
class ComparisonExpression;
class BinaryExpression;
class UnaryExpression;
class DereferenceExpression;
class AddressofExpression;
class IfExpression;
class MatchExpression;
class NewExpression;
class BlockExpression;
class FnCallExpression;
class IntrinsicCall;
class CompoundInitializerExpr;
class FieldAccessExpression;
class LiteralExpression;
class VarAccessExpression;
class TypecastExpression;
class ReturnExpression;
class YieldExpression;
class IndexExpression;

//////////////////////////////////////////////////////////////////////

class Statement;
class ExprStatement;
class AssignmentStatement;

//////////////////////////////////////////////////////////////////////

class TypeDeclStatement;
class TraitDeclaration;
class ImplDeclaration;
class VarDeclStatement;
class FunDeclStatement;

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

  // Statements

  virtual void VisitAssignment(AssignmentStatement* node) = 0;

  virtual void VisitExprStatement(ExprStatement* node) = 0;

  // Declarations

  virtual void VisitTypeDecl(TypeDeclStatement* node) = 0;

  virtual void VisitVarDecl(VarDeclStatement* node) = 0;

  virtual void VisitFunDecl(FunDeclStatement* node) = 0;

  virtual void VisitTraitDecl(TraitDeclaration* node) = 0;

  virtual void VisitImplDecl(ImplDeclaration* node) = 0;

  // Patterns

  virtual void VisitBindingPat(BindingPattern* node) = 0;

  virtual void VisitDiscardingPat(DiscardingPattern* node) = 0;

  virtual void VisitLiteralPat(LiteralPattern* node) = 0;

  virtual void VisitStructPat(StructPattern* node) = 0;

  virtual void VisitVariantPat(VariantPattern* node) = 0;

  // Expressions

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
