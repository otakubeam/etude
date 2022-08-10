#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/expressions.hpp>

#include <types/repr/struct_type.hpp>
#include <types/repr/fn_type.hpp>

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

class StructDeclStatement : public Statement {
 public:
  StructDeclStatement(lex::Token name, std::vector<lex::Token> field_names,
                      std::vector<types::Type*> field_types)
      : name_{name}, field_names_{field_names}, field_types_{field_types} {
    type_ = new types::StructType{name.GetName()};
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitStructDecl(this);
  }

  // Assumes offset in SBValue widths
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

  lex::Token name_;
  types::StructType* type_;

  std::vector<lex::Token> field_names_;
  std::vector<types::Type*> field_types_;
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

  FunDeclStatement(lex::Token name, types::Type* return_type,
                   std::vector<FormalParam> formals, BlockExpression* block)
      : name_{name}, formals_{std::move(formals)}, block_{block} {
    InitFnType(return_type);
  }

  void InitFnType(types::Type* return_type) {
    std::vector<types::Type*> just_arg_types;

    for (auto fm : formals_) {
      just_arg_types.push_back(fm.type);
    }

    type_ = new types::FnType{std::move(just_arg_types), return_type};
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFunDecl(this);
  }

  lex::Token name_;
  types::FnType* type_ = nullptr;

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
