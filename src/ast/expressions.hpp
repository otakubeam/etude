#pragma once

#include <ast/scope/context.hpp>
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
    return &types::builtin_bool;
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
    FMT_ASSERT(type_, "Type is not set");
    return type_;
  };

  virtual lex::Location GetLocation() override {
    return operator_.location;
  }

  Expression* left_;
  lex::Token operator_;
  Expression* right_;

  types::Type* type_ = nullptr;

  bool is_pointer_arithmetic_ = false;
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
    return operand_->GetType();
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

  virtual bool IsDirect() override {
    return false;
  }

  virtual lex::Location GetLocation() override {
    return star_.location;
  }

  lex::Token star_;

  // The pointer expression
  Expression* operand_;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class AddressofExpression : public Expression {
 public:
  AddressofExpression(lex::Token ampersand, LvalueExpression* operand)
      : ampersand_{ampersand}, operand_{operand} {
    // Transform &*unit -> unit (it works like that in C)
    if (auto op = dynamic_cast<DereferenceExpression*>(operand_)) {
      operand_ = op->operand_;
    }
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

  /*Lvalue*/ Expression* operand_;

  // Mabye embed and save allocation
  types::PointerType* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class FnCallExpression : public Expression {
 public:
  // No-name, e.g. vec[10]()
  FnCallExpression(lex::Location call_site, Expression* callable,
                   std::vector<Expression*> arguments)
      : call_site_(call_site), callable_{callable}, arguments_{arguments} {
  }

  // Named function call: foo(), struct.field(), etc...
  FnCallExpression(lex::Token name, Expression* callable,
                   std::vector<Expression*> arguments)
      : call_site_(name.location),
        fn_name_{name.GetName()},
        callable_{callable},
        arguments_{arguments} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFnCall(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(expression_type, "Typechecking fault");
    return expression_type;
  };

  std::string_view GetFunctionName() {
    return fn_name_;
  };

  virtual lex::Location GetLocation() override {
    return call_site_;
  }

  lex::Location call_site_;

  // May be absent
  std::string_view fn_name_;

  Expression* callable_;

  std::vector<Expression*> arguments_;

  bool is_native_call_ = false;
  bool is_tail_call_ = false;

  types::Type* expression_type = nullptr;

  ast::scope::ScopeLayer* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

// At least for now let's call it that
class CompoundInitializerExpr : public Expression {
 public:
  CompoundInitializerExpr(lex::Token struct_name,
                               std::vector<Expression*> values)
      : struct_name_{struct_name}, values_{values} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitCompoundInitalizer(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Oh-huh!");
    return type_;
  };

  std::string_view GetStructName() {
    return struct_name_.GetName();
  };

  virtual lex::Location GetLocation() override {
    return struct_name_.location;
  }

  lex::Token struct_name_;

  std::vector<Expression*> values_;

  types::Type* type_ = nullptr;

  ast::scope::ScopeLayer* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class FieldAccessExpression : public Expression {
 public:
  FieldAccessExpression(lex::Token field_name, Expression* lvalue)
      : struct_expression_{lvalue}, field_name_{field_name} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFieldAccess(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Returning unassigned type");
    return type_;
  };

  std::string_view GetFieldName() {
    return field_name_.GetName();
  }

  virtual lex::Location GetLocation() override {
    return field_name_.location;
  }

  // This can be an Identifier or or result of
  // indexing an array, or of a field access.
  Expression* struct_expression_;

  types::Type* type_ = nullptr;

  lex::Token field_name_;

  ast::scope::ScopeLayer* layer_ = nullptr;
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
    return final_->GetType();
  };

  virtual lex::Location GetLocation() override {
    return curly_brace_.location;
  }

  lex::Token curly_brace_{};

  std::vector<Statement*> stmts_;

  Expression* final_;

  ast::scope::ScopeLayer* layer_ = nullptr;
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
    return true_branch_->GetType();
  };

  virtual lex::Location GetLocation() override {
    return condition_->GetLocation();
  }

  Expression* condition_;
  Expression* true_branch_;
  Expression* false_branch_;
};

//////////////////////////////////////////////////////////////////////

class NewExpression : public LvalueExpression {
 public:
  NewExpression(lex::Token new_token, Expression* allocation_size,
                types::Type* underlying)
      : new_token_{new_token},
        allocation_size_{allocation_size},
        underlying_{underlying},
        type_{new types::PointerType{underlying_}} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitNew(this);
  }

  virtual types::Type* GetType() override {
    return type_;
  };

  virtual lex::Location GetLocation() override {
    return new_token_.location;
  }

  virtual bool IsDirect() override {
    return false;
  }

  lex::Token new_token_{};

  Expression* allocation_size_ = nullptr;

  types::Type* underlying_ = nullptr;

  types::Type* type_ = nullptr;
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
    return type_;
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

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Type field is not set!");
    return type_;
  };

  std::string_view GetName() {
    return name_.GetName();
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  lex::Token name_;

  types::Type* type_ = nullptr;

  ast::scope::ScopeLayer* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class TypecastExpression : public Expression {
 public:
  TypecastExpression(Expression* expr, lex::Token flowy_arrow,
                     types::Type* dest)
      : expr_{expr}, flowy_arrow_{flowy_arrow}, type_{dest} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitTypecast(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Type field is not set!");
    return type_;
  };

  virtual lex::Location GetLocation() override {
    return flowy_arrow_.location;
  }

  Expression* expr_ = nullptr;

  lex::Token flowy_arrow_;

  types::Type* type_ = nullptr;
};
