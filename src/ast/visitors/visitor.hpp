#pragma once

#include <rt/base_object.hpp>

//////////////////////////////////////////////////////////////////////

class TreeNode;

class Expression;
//---------------
class ComparisonExpression;
class BinaryExpression;
class UnaryExpression;
class FnCallExpression;
class LiteralExpression;

//---------------
class Statement;

class IfStatement;
class ExprStatement;
class BlockStatement;

class YieldStatement;
class ReturnStatement;

class VarDeclStatement;
class FunDeclStatement;

class Visitor {
 public:
  virtual ~Visitor() = default;

  // Statements

  virtual void VisitIf(IfStatement* node) = 0;

  virtual void VisitStatement(Statement* node) = 0;

  virtual void VisitYield(YieldStatement* node) = 0;

  virtual void VisitReturn(ReturnStatement* node) = 0;

  virtual void VisitVarDecl(VarDeclStatement* node) = 0;

  virtual void VisitFunDecl(FunDeclStatement* node) = 0;

  virtual void VisitExprStatement(ExprStatement* node) = 0;

  virtual void VisitBlockStatement(BlockStatement* node) = 0;

  // Expressions

  virtual void VisitExpression(Expression* node) = 0;

  virtual void VisitComparison(ComparisonExpression* node) = 0;

  virtual void VisitBinary(BinaryExpression* node) = 0;

  virtual void VisitUnary(UnaryExpression* node) = 0;

  virtual void VisitFnCall(FnCallExpression* node) = 0;

  virtual void VisitLiteral(LiteralExpression* node) = 0;
};

//////////////////////////////////////////////////////////////////////
