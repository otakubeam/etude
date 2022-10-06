#include <vm/codegen/compiler.hpp>

#include <vm/debug/disassember.hpp>
#include <vm/debug/debugger.hpp>

#include <vm/instr_translator.hpp>

#include <vm/interpreter.hpp>

#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:simple", "[vm:codgen]") {
  char stream[] =
      "   fun main() Int {     "
      "      var a = 5;       "
      "      var b = &a;      "
      "      *b = 6;          "
      "      a                "
      "   }                   ";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};
  auto stmt = p.ParseFunDeclStatement();

  types::check::TypeChecker tchk;
  tchk.Eval(stmt);

  vm::codegen::Compiler compiler;
  auto elf = compiler.Compile(stmt);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  CHECK(debugger.RunToTheEnd().as_int == 6);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:simple:mul", "[vm:codgen]") {
  char stream[] =
      "    fun mul(a: Int, b: Int) Int {              "
      "       if a == 1 {                             "
      "          b                                    "
      "       } else {                                "
      "          b + mul(a-1, b)                      "
      "       }                                       "
      "    }                                          "
      "                                               "
      "    fun main() Int {                           "
      "        mul(3, 5)                              "
      "    }                                          "
      "                                               ";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  vm::codegen::Compiler compiler;
  // vm::debug::Disassembler d;

  auto stmt1 = p.ParseFunDeclStatement();
  tchk.Eval(stmt1);
  auto elf = compiler.Compile(stmt1);

  auto stmt2 = p.ParseFunDeclStatement();
  tchk.Eval(stmt2);
  elf += compiler.Compile(stmt2);

  // d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  CHECK(debugger.RunToTheEnd().as_int == 15);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:struct", "[vm:codgen]") {
  char stream[] =
      "  struct Str {                                   "
      "     count: Int,                                 "
      "     ismodified: Bool,                           "
      "  };                                             "
      "                                                 "
      "  fun main() Bool {                              "
      "     var instance = Str:{4, false};              "
      "     instance.ismodified = true;                 "
      "     instance.ismodified                         "
      "  }                                              "
      "                                                 ";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto stmt1 = p.ParseStatement();
  tchk.Eval(stmt1);
  compiler.Compile(stmt1);

  auto stmt2 = p.ParseStatement();
  tchk.Eval(stmt2);
  auto elf = compiler.Compile(stmt2);

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  CHECK(debugger.RunToTheEnd().as_bool == true);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:struct:nested", "[vm:codgen]") {
  char stream[] =
      "  struct Inner {                                   "
      "     fieldOne: Int,                                "
      "     fieldTwo: Int,                                "
      "  };                                               "
      "                                                   "
      "  struct Str {                                     "
      "     count: Int,                                   "
      "     inner: Inner,                                 "
      "     ismodified: Bool,                             "
      "  };                                               "
      "                                                   "
      "  fun main() Bool {                                "
      "     var instance = Str:{4, Inner:{0,1}, false};   "
      "     instance.inner.fieldTwo = 123;                "
      "     instance.ismodified  = true;                  "
      "     instance.ismodified                           "
      "  }                                                "
      "                                                   ";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto stmt1 = p.ParseStatement();
  tchk.Eval(stmt1);
  compiler.Compile(stmt1);

  auto stmt2 = p.ParseStatement();
  tchk.Eval(stmt2);
  compiler.Compile(stmt2);

  auto stmt3 = p.ParseStatement();
  tchk.Eval(stmt3);
  auto elf = compiler.Compile(stmt3);

  // d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  CHECK(debugger.RunToTheEnd().as_bool == true);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:struct:tree", "[vm:codgen]") {
  char stream[] =
      "                                                  "
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
      "  fun main() Bool {                                "
      "     var tree = Tree:{unit, unit, 3};              "
      "     var treePtr = &tree;                          "
      "     binSearchInt(treePtr, 3)                      "
      "  }                                                ";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto stmt1 = p.ParseStatement();
  tchk.Eval(stmt1);
  compiler.Compile(stmt1);

  auto stmt2 = p.ParseStatement();
  tchk.Eval(stmt2);
  auto elf = compiler.Compile(stmt2);

  auto stmt3 = p.ParseStatement();
  tchk.Eval(stmt3);
  elf += compiler.Compile(stmt3);

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  CHECK(debugger.StepToTheEnd().as_bool == true);
}
