
#include <types/check/type_checker.hpp>

#include <vm/codegen/compiler.hpp>
#include <vm/interpreter.hpp>

#include <parse/parser.hpp>
#include <lex/lexer.hpp>

// Finally,
#include <catch2/catch.hpp>

extern bool print_debug_info;

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:tree:simple", "[vm:codegen:tree]") {
  char stream[] =
      "{                                                  "
      "   struct Tree {                                   "
      "      left: *Tree,                                 "
      "      right: *Tree,                                "
      "      value: Int,                                  "
      "   };                                              "
      "                                                   "
      "   fun binSearchInt(tree: *Tree, val: Int) Bool {  "
      "       if isNull(tree) {                           "
      "           return false;                           "
      "       };                                          "
      "                                                   "
      "       var nodeValue = (*tree).value;              "
      "                                                   "
      "       if nodeValue == val {                       "
      "          return true;                             "
      "       };                                          "
      "                                                   "
      "       var subtree = if nodeValue < val {          "
      "           (*tree).left                            "
      "       } else {                                    "
      "           (*tree).right                           "
      "       };                                          "
      "                                                   "
      "       binSearchInt(subtree, val)                  "
      "   }                                               "
      "                                                   "
      "   var tree = Tree:{unit, unit, 3};                "
      "   var treePtr = &tree;                            "
      "                                                   "
      "   binSearchInt(treePtr, 3)                        "
      "}                                                  ";

  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();

  types::check::TypeChecker tchk;
  CHECK_NOTHROW(tchk.Eval(expr));

  vm::codegen::Compiler c;
  auto res = c.CompileScript(expr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(*res) == 1);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:tree:nonexist", "[vm:codegen:tree]") {
  char stream[] =
      "{                                                  "
      "   struct Tree {                                   "
      "      left: *Tree,                                 "
      "      right: *Tree,                                "
      "      value: Int,                                  "
      "   };                                              "
      "                                                   "
      "   fun binSearchInt(tree: *Tree, val: Int) Bool {  "
      "       if isNull(tree) {                           "
      "           return false;                           "
      "       };                                          "
      "                                                   "
      "       var nodeValue = (*tree).value;              "
      "                                                   "
      "       if nodeValue == val {                       "
      "          return true;                             "
      "       };                                          "
      "                                                   "
      "       var subtree = if nodeValue < val {          "
      "           (*tree).left                            "
      "       } else {                                    "
      "           (*tree).right                           "
      "       };                                          "
      "                                                   "
      "       binSearchInt(subtree, val)                  "
      "   }                                               "
      "                                                   "
      "   var tree = Tree:{unit, unit, 3};                "
      "   var treePtr = &tree;                            "
      "                                                   "
      "   binSearchInt(treePtr, 4)                        "
      "}                                                  ";

  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();

  types::check::TypeChecker tchk;
  CHECK_NOTHROW(tchk.Eval(expr));

  vm::codegen::Compiler c;
  auto res = c.CompileScript(expr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(*res) == 0);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:tree:change-node", "[vm:codegen:tree]") {
  char stream[] =
      "{                                                  "
      "   struct Tree {                                   "
      "      left: *Tree,                                 "
      "      right: *Tree,                                "
      "      value: Int,                                  "
      "   };                                              "
      "                                                   "
      "   fun binSearchInt(tree: *Tree, val: Int) *Tree { "
      "       if isNull(tree) {                           "
      "           return tree;                            "
      "       };                                          "
      "                                                   "
      "       var nodeValue = (*tree).value;              "
      "                                                   "
      "       if nodeValue == val {                       "
      "          return tree;                             "
      "       };                                          "
      "                                                   "
      "       var subtree = if nodeValue < val {          "
      "           (*tree).left                            "
      "       } else {                                    "
      "           (*tree).right                           "
      "       };                                          "
      "                                                   "
      "       binSearchInt(subtree, val)                  "
      "   }                                               "
      "                                                   "
      "   var tree = Tree:{unit, unit, 3};                "
      "   var treePtr = &tree;                            "
      "                                                   "
      "   var foundNode = binSearchInt(treePtr, 3);       "
      "   (*foundNode).value = 5;                         "
      "                                                   "
      "   # Assert that node did in fact changed \n       "
      "                                                   "
      "   tree.value                                      "
      "}                                                  ";

  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();

  types::check::TypeChecker tchk;
  CHECK_NOTHROW(tchk.Eval(expr));

  vm::codegen::Compiler c;
  auto res = c.CompileScript(expr);

  print_debug_info = true;
  for (auto r : *res) {
    r.Print();
  }

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(*res) == 5);
}

//////////////////////////////////////////////////////////////////////
