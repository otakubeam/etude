#pragma once

#include <ast/visitors/visitor.hpp>

#include <lex/location.hpp>

//////////////////////////////////////////////////////////////////////

struct Attribute {
  std::string_view value;
  Attribute* next = nullptr;
};

//////////////////////////////////////////////////////////////////////

class TreeNode {
 public:
  virtual void Accept(Visitor* visitor) = 0;

  virtual lex::Location GetLocation() = 0;

  virtual ~TreeNode() = default;

  template <typename T>
  T* as() {
    return dynamic_cast<T*>(this);
  }
};

//////////////////////////////////////////////////////////////////////
