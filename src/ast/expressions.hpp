#pragma once

#include <ast/syntax_tree.hpp>

#include <types/type.hpp>

#include <lex/token.hpp>

#include <vector>

//////////////////////////////////////////////////////////////////////

class Expression : public TreeNode {
 public:
  virtual void Accept(Visitor* /* visitor */){};

  virtual types::Type* GetType() = 0;
};

//////////////////////////////////////////////////////////////////////

// Identifier, Named entity
class LvalueExpression : public Expression {
 public:
  virtual int GetAddress() = 0;
};

//////////////////////////////////////////////////////////////////////

class ComparisonExpression : public Expression {
 public:
  ComparisonExpression(Expression* left, lex::Token op, Expression* right)
      : left_{left}, operator_(op), right_{right} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitComparison(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  Expression* left_;
  lex::Token operator_;
  Expression* right_;
};

//////////////////////////////////////////////////////////////////////

class BinaryExpression : public Expression {
 public:
  BinaryExpression(Expression* left, lex::Token op, Expression* right)
      : left_{left}, operator_(op), right_{right} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitBinary(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  Expression* left_;
  lex::Token operator_;
  Expression* right_;
};

//////////////////////////////////////////////////////////////////////

class UnaryExpression : public Expression {
 public:
  UnaryExpression(lex::Token op, Expression* operand)
      : operator_(op), operand_{operand} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitUnary(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  lex::Token operator_;
  Expression* operand_;
};

//////////////////////////////////////////////////////////////////////

class FnCallExpression : public Expression {
 public:
  FnCallExpression(lex::Token fn_name, std::vector<Expression*> arguments)
      : fn_name_(fn_name), arguments_{arguments} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFnCall(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  lex::Token fn_name_;
  std::vector<Expression*> arguments_;
};

//////////////////////////////////////////////////////////////////////

// At least for now let's call it that
class StructConstructionExpression : public Expression {
 public:
  StructConstructionExpression(lex::Token struct_name,
                               std::vector<Expression*> values)
      : struct_name_{struct_name}, values_{values} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitStructConstruction(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Oh-huh!");
    return type_;
  };

  lex::Token struct_name_;
  std::vector<Expression*> values_;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class FieldAccessExpression : public LvalueExpression {
 public:
  FieldAccessExpression(lex::Token struct_name, lex::Token field_name,
                        LvalueExpression* lvalue)
      : struct_expression_{lvalue},
        struct_name_{struct_name},
        field_name_{field_name} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFieldAccess(this);
  }

  virtual int GetAddress() override {
    return address_;
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Returning unassigned type");
    return type_;
  };

  // This can be an Identifier or result of a function call
  // or result of indexing an array, or of a field access.
  LvalueExpression* struct_expression_;

  types::Type* type_ = nullptr;

  // TODO: deprecate, remove
  lex::Token struct_name_;

  lex::Token field_name_;

  int address_ = 0;
};

//////////////////////////////////////////////////////////////////////

class BlockExpression : public Expression {
 public:
  BlockExpression(std::vector<Statement*> stmts, Expression* final = nullptr)
      : stmts_{stmts}, final_{final} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitBlock(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  std::vector<Statement*> stmts_;
  Expression* final_;
};

//////////////////////////////////////////////////////////////////////

class IfExpression : public Expression {
 public:
  IfExpression(Expression* condition, Expression* true_branch,
               Expression* false_branch = nullptr)
      : condition_{condition},
        true_branch_{true_branch},
        false_branch_{false_branch} {
    if (false_branch_ == nullptr) {
      false_branch_ = new BlockExpression{{}};
    }
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitIf(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  Expression* condition_;
  Expression* true_branch_;
  Expression* false_branch_;
};

//////////////////////////////////////////////////////////////////////

class LiteralExpression : public Expression {
 public:
  LiteralExpression(lex::Token token) : token_{token} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitLiteral(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  types::Type* type_ = nullptr;
  lex::Token token_{};
};

//////////////////////////////////////////////////////////////////////

class VarAccessExpression : public LvalueExpression {
 public:
  VarAccessExpression(lex::Token name) : name_{name} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitVarAccess(this);
  }

  virtual int GetAddress() override {
    return address_;
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Type field is not set!");
    return type_;
  };

  lex::Token name_{};

  types::Type* type_ = nullptr;

  int address_ = 0;
};

//////////////////////////////////////////////////////////////////////
