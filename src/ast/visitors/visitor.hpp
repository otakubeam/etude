#pragma once

//////////////////////////////////////////////////////////////////////

class Expression;
class ComparisonExpression;
class BinaryExpression;
class UnaryExpression;
class DereferenceExpression;
class AddressofExpression;
class IfExpression;
class NewExpression;
class BlockExpression;
class FnCallExpression;
class StructConstructionExpression;
class FieldAccessExpression;
class LiteralExpression;
class VarAccessExpression;
class TypecastExpression;

//////////////////////////////////////////////////////////////////////

class Statement;
class ExprStatement;
class YieldStatement;
class ReturnStatement;
class StructDeclStatement;
class VarDeclStatement;
class AssignmentStatement;
class FunDeclStatement;

//////////////////////////////////////////////////////////////////////

class Visitor {
 public:
  virtual ~Visitor() = default;

  // Statements

  virtual void VisitYield(YieldStatement* node) = 0;

  virtual void VisitReturn(ReturnStatement* node) = 0;

  virtual void VisitStructDecl(StructDeclStatement* node) = 0;

  virtual void VisitVarDecl(VarDeclStatement* node) = 0;

  virtual void VisitFunDecl(FunDeclStatement* node) = 0;

  virtual void VisitAssignment(AssignmentStatement* node) = 0;

  virtual void VisitExprStatement(ExprStatement* node) = 0;

  // Expressions

  virtual void VisitComparison(ComparisonExpression* node) = 0;

  virtual void VisitBinary(BinaryExpression* node) = 0;

  virtual void VisitUnary(UnaryExpression* node) = 0;

  virtual void VisitDeref(DereferenceExpression* node) = 0;

  virtual void VisitAddressof(AddressofExpression* node) = 0;

  virtual void VisitIf(IfExpression* node) = 0;

  virtual void VisitNew(NewExpression* node) = 0;

  virtual void VisitBlock(BlockExpression* node) = 0;

  virtual void VisitFnCall(FnCallExpression* node) = 0;

  virtual void VisitStructConstruction(StructConstructionExpression* node) = 0;

  virtual void VisitFieldAccess(FieldAccessExpression* node) = 0;

  virtual void VisitVarAccess(VarAccessExpression* node) = 0;

  virtual void VisitLiteral(LiteralExpression* node) = 0;

  virtual void VisitTypecast(TypecastExpression* node) = 0;
};

//////////////////////////////////////////////////////////////////////
