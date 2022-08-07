#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/expressions.hpp>

#include <lex/token.hpp>

#include <vector>

//////////////////////////////////////////////////////////////////////

class Statement : public TreeNode {
 public:
  virtual void Accept(Visitor* /* visitor */){};
};

//////////////////////////////////////////////////////////////////////

class ExprStatement : public Statement {
 public:
  ExprStatement(Expression* expr) : expr_{expr} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitExprStatement(this);
  }

  Expression* expr_;
};

//////////////////////////////////////////////////////////////////////

// Also stands for symbol
class StructDeclStatement : public Statement {
 public:
  StructDeclStatement(lex::Token name, std::vector<lex::Token> field_names)
      : name_{name}, field_names_{field_names} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitStructDecl(this);
  }

  lex::Token name_;

  // TODO:
  // types::StrcutType* type_;

  // Supposes offset in SBValue types
  size_t OffsetOf(std::string name) const {
    for (size_t i = 0; i < field_names_.size(); i++) {
      auto& t = field_names_[i];
      if (t.GetName() == name) {
        return i;
      }
    }

    // In well types program should not happen
    FMT_ASSERT(false, "No offset");
  }

  std::vector<lex::Token> field_names_;
  // std::vector<types::Type*> field_types_;
};

//////////////////////////////////////////////////////////////////////

class VarDeclStatement : public Statement {
 public:
  VarDeclStatement(VarAccessExpression* lvalue, Expression* value)
      : lvalue_{lvalue}, value_{value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitVarDecl(this);
  }

  VarAccessExpression* lvalue_;
  Expression* value_;
};

//////////////////////////////////////////////////////////////////////

// fun f(a1, a2, a3) { }

class FunDeclStatement : public Statement {
 public:
  struct FormalParam {
    lex::Token ident;
    types::Type* type;
  };

  FunDeclStatement(lex::Token name, types::FnType* type,
                   std::vector<FormalParam> formals, BlockExpression* block)
      : name_{name}, type_{type}, formals_{std::move(formals)}, block_{block} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFunDecl(this);
  }

  lex::Token name_;
  types::FnType* type_;

  std::vector<FormalParam> formals_;
  BlockExpression* block_;
};

//////////////////////////////////////////////////////////////////////

class ReturnStatement : public Statement {
 public:
  ReturnStatement(Expression* return_value) : return_value_{return_value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitReturn(this);
  }

  Expression* return_value_;
};

//////////////////////////////////////////////////////////////////////

class YieldStatement : public Statement {
 public:
  YieldStatement(Expression* yield_value) : yield_value_{yield_value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitYield(this);
  }

  Expression* yield_value_;
};

//////////////////////////////////////////////////////////////////////
