#pragma once

#include <ast/syntax_tree.hpp>

#include <types/repr/pointer_type.hpp>
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
  // TODO: use StorageLocation interface instead
  // This will be useful for implementing different backends
  virtual int GetAddress() = 0;

  virtual bool IsDirect() {
    // True for compile-time expressions
    // But not for pointers
    return true;
  }
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

  virtual lex::Location GetLocation() override {
    return operator_.location;
  }

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

  virtual lex::Location GetLocation() override {
    return operator_.location;
  }

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

  virtual lex::Location GetLocation() override {
    return operator_.location;
  }

  lex::Token operator_;
  Expression* operand_;
};

//////////////////////////////////////////////////////////////////////

class DereferenceExpression : public LvalueExpression {
 public:
  DereferenceExpression(lex::Token star, Expression* operand)
      : star_{star}, operand_{operand} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitDeref(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Typechecker fault!");
    return type_;
  };

  virtual int GetAddress() override {
    FMT_ASSERT(false,
               "Cannot take the address of "
               "pointer dereference at compile-time\n");
  }

  virtual bool IsDirect() override {
    return false;
  }

  virtual lex::Location GetLocation() override {
    return star_.location;
  }

  lex::Token star_;
  // The pointer expression
  Expression* operand_;

  int address_ = 0;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class AddressofExpression : public Expression {
 public:
  AddressofExpression(lex::Token ampersand, LvalueExpression* operand)
      : ampersand_{ampersand}, operand_{operand} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitAddressof(this);
  }

  virtual types::Type* GetType() override {
    return type_;
  };

  virtual lex::Location GetLocation() override {
    return ampersand_.location;
  }

  lex::Token ampersand_;

  LvalueExpression* operand_;

  // Mabye embed and save allocation
  types::PointerType* type_ = nullptr;
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

  std::string GetFunctionName() {
    return fn_name_.GetName();
  };

  virtual lex::Location GetLocation() override {
    return fn_name_.location;
  }

  lex::Token fn_name_;

  std::vector<Expression*> arguments_;

  bool is_native_call_ = false;
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

  std::string GetStructName() {
    return struct_name_.GetName();
  };

  virtual lex::Location GetLocation() override {
    return struct_name_.location;
  }

  lex::Token struct_name_;

  std::vector<Expression*> values_;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class FieldAccessExpression : public LvalueExpression {
 public:
  FieldAccessExpression(lex::Token field_name, LvalueExpression* lvalue)
      : struct_expression_{lvalue}, field_name_{field_name} {
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

  virtual bool IsDirect() override {
    return struct_expression_->IsDirect();
  }

  std::string GetFieldName() {
    return field_name_.GetName();
  }

  virtual lex::Location GetLocation() override {
    return field_name_.location;
  }

  // This can be an Identifier or or result of 
  // indexing an array, or of a field access.
  LvalueExpression* struct_expression_;

  types::Type* type_ = nullptr;

  lex::Token field_name_;

  bool is_direct_ = true;

  int address_ = 0;
};

//////////////////////////////////////////////////////////////////////

class BlockExpression : public Expression {
 public:
  BlockExpression(lex::Token curly_brace, std::vector<Statement*> stmts,
                  Expression* final)
      : curly_brace_{curly_brace}, stmts_{stmts}, final_{final} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitBlock(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  virtual lex::Location GetLocation() override {
    return curly_brace_.location;
  }

  lex::Token curly_brace_{};

  std::vector<Statement*> stmts_;

  Expression* final_;
};

//////////////////////////////////////////////////////////////////////

class IfExpression : public Expression {
 public:
  IfExpression(Expression* condition, Expression* true_branch,
               Expression* false_branch)
      : condition_{condition},
        true_branch_{true_branch},
        false_branch_{false_branch} {
    if (!false_branch_) {
      false_branch_ = new BlockExpression{{}, {}, nullptr};
    }
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitIf(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(false, "Unreachable!");
    return nullptr;
  };

  virtual lex::Location GetLocation() override {
    return condition_->GetLocation();
  }

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

  virtual lex::Location GetLocation() override {
    return token_.location;
  }

  types::Type* type_ = nullptr;

  lex::Token token_;
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

  std::string GetName() {
    return name_.GetName();
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  lex::Token name_;

  types::Type* type_ = nullptr;

  int address_ = 0;
};

//////////////////////////////////////////////////////////////////////
