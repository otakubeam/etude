#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/expressions.hpp>

#include <lex/token.hpp>

#include <vector>

//////////////////////////////////////////////////////////////////////

class Pattern : public TreeNode {
 public:
  virtual void Accept(Visitor* /* visitor */){};
};

//////////////////////////////////////////////////////////////////////

class BindingPattern : public Pattern {
 public:
  BindingPattern(lex::Token name) : name_{name} {
  }

  virtual void Accept(Visitor* visitor) {
    visitor->VisitBindingPat(this);
  };

  virtual lex::Location GetLocation() {
    return name_.location;
  };

  lex::Token name_;
  types::Type* type_ = nullptr;
  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class LiteralPattern : public Pattern {
 public:
  LiteralPattern(LiteralExpression* pat) : pat_(pat) {
  }

  virtual void Accept(Visitor* visitor) {
    visitor->VisitLiteralPat(this);
  };

  virtual lex::Location GetLocation() {
    return pat_->GetLocation();
  };

  LiteralExpression* pat_;
};

//////////////////////////////////////////////////////////////////////

class VariantPattern : public Pattern {
 public:
  VariantPattern(lex::Token name, Pattern* inner)
      : name_{name}, inner_pat_(inner) {
  }

  virtual void Accept(Visitor* visitor) {
    visitor->VisitVariantPat(this);
  };

  virtual lex::Location GetLocation() {
    return name_.location;
  };

  lex::Token name_;
  Pattern* inner_pat_ = nullptr;
  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class StructPattern;

//////////////////////////////////////////////////////////////////////
