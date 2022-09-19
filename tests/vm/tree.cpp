
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

  // print_debug_info = true;
  // for (auto r : *res) {
  //   r.Print();
  // }

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(*res) == 5);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:tree:tree-sum", "[vm:codegen:tree]") {
  char stream[] =
      "{                                                    "
      "  struct Tree {                                      "
      "     left: *Tree,                                    "
      "     right: *Tree,                                   "
      "     value: Int,                                     "
      "  };                                                 "
      "                                                     "
      "  var tr1 = Tree:{unit, unit, 1};                    "
      "  var tr4 = Tree:{unit, unit, 4};                    "
      "  var tr3 = Tree:{unit, &tr4, 3};                    "
      "  var tr2 = Tree:{&tr1, &tr3, 2};                    "
      "                                                     "
      "  fun countTreeSum(tree: *Tree) Int {                "
      "      if isNull(tree) {                              "
      "          return 0;                                  "
      "      };                                             "
      "                                                     "
      "      (*tree).value + countTreeSum((*tree).left)     "
      "                    + countTreeSum((*tree).right)    "
      "  }                                                  "
      "                                                     "
      "  countTreeSum(&tr2)                                 "
      "}                                                    ";

  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();

  types::check::TypeChecker tchk;
  CHECK_NOTHROW(tchk.Eval(expr));

  vm::codegen::Compiler c;
  auto res = c.CompileScript(expr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(*res) == 10);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:tree:insert", "[vm:codegen:tree]") {
  char stream[] =
      "{                                                    "
      "  struct Tree {                                      "
      "     left: *Tree,                                    "
      "     right: *Tree,                                   "
      "     value: Int,                                     "
      "  };                                                 "
      "                                                     "
      "  var tr1 = Tree:{unit, unit, 1};                    "
      "                                                     "
      "  fun insertNewValue(tree: *Tree, node: *Tree) Bool {"
      "      var nodeValue = (*node).value;                 "
      "      var treeValue = (*tree).value;                 "
      "                                                     "
      "      if treeValue == nodeValue {                    "
      "         return false;                               "
      "      };                                             "
      "                                                     "
      "      var candidate = if nodeValue < treeValue {     "
      "         &(*tree).left                               "
      "      } else {                                       "
      "         &(*tree).right                              "
      "      };                                             "
      "                                                     "
      "      if isNull(*candidate) {                        "
      "         *candidate = node;                          "
      "         true                                        "
      "      } else {                                       "
      "         insertNewValue(*candidate, node)            "
      "      }                                              "
      "  }                                                  "
      "                                                     "
      "  var tr5 = Tree:{unit, unit, 5};                    "
      "  var tr4 = Tree:{unit, unit, 4};                    "
      "  var tr3 = Tree:{unit, unit, 3};                    "
      "  var tr2 = Tree:{unit, unit, 2};                    "
      "                                                     "
      "  insertNewValue(&tr1, &tr5);                        "
      "  insertNewValue(&tr1, &tr3);                        "
      "  insertNewValue(&tr1, &tr2);                        "
      "  insertNewValue(&tr1, &tr4);                        "
      "  insertNewValue(&tr1, &tr4)                         "
      "}                                                    ";

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

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(*res) == 0);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:tree:red-black", "[vm:codegen:tree]") {
  char stream[] =
      "{                                                    "
      "  struct Tree {                                      "
      "     left: *Tree,                                    "
      "     right: *Tree,                                   "
      "     value: Int,                                     "
      "  };                                                 "
      "                                                     "
      "  var tr1 = Tree:{unit, unit, 1};                    "
      "                                                     "
      "  fun insertNewValue(tree: *Tree, node: *Tree) Bool {"
      "      var nodeValue = (*node).value;                 "
      "      var treeValue = (*tree).value;                 "
      "                                                     "
      "      if treeValue == nodeValue {                    "
      "         return false;                               "
      "      };                                             "
      "                                                     "
      "      var candidate = if nodeValue < treeValue {     "
      "         &(*tree).left                               "
      "      } else {                                       "
      "         &(*tree).right                              "
      "      };                                             "
      "                                                     "
      "      if isNull(*candidate) {                        "
      "         *candidate = node;                          "
      "         true                                        "
      "      } else {                                       "
      "         insertNewValue(*candidate, node)            "
      "      }                                              "
      "  }                                                  "
      "                                                     "
      "  var tr5 = Tree:{unit, unit, 5};                    "
      "  var tr4 = Tree:{unit, unit, 4};                    "
      "  var tr3 = Tree:{unit, unit, 3};                    "
      "  var tr2 = Tree:{unit, unit, 2};                    "
      "                                                     "
      "  insertNewValue(&tr1, &tr5);                        "
      "  insertNewValue(&tr1, &tr3);                        "
      "  insertNewValue(&tr1, &tr2);                        "
      "  insertNewValue(&tr1, &tr4);                        "
      "  insertNewValue(&tr1, &tr4)                         "
      "}                                                    ";

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

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(*res) == 0);
}

//////////////////////////////////////////////////////////////////////
