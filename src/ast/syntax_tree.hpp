#pragma once

#include <ast/visitors/visitor.hpp>

#include <lex/location.hpp>

//////////////////////////////////////////////////////////////////////

struct Attribute {
  std::string_view value;
  Attribute* next = nullptr;

  bool FindAttr(std::string_view attr) {
    return attr == value || (next && next->FindAttr(attr));
  }
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
