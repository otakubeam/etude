#include <vm/codegen/compiler.hpp>

#include <vm/debug/disassember.hpp>
#include <vm/debug/debugger.hpp>

#include <vm/instr_translator.hpp>

#include <vm/interpreter.hpp>

#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>

#include <types/instantiate/instantiator.hpp>
#include <types/check/algorithm_w.hpp>

#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:simple", "[vm:codgen]") {
  types::Type::type_store.clear();

  char stream[] =
      "   fun main = {        "
      "      var a = 5;       "
      "      var b = &a;      "
      "      *b = 6;          "
      "      a                "
      "   };                  ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(0)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  // vm::debug::Debugger debugger;
  // debugger.Load(std::move(elf));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:poly:swap", "[vm:codgen]") {
  types::Type::type_store.clear();

  char stream[] =
      "    fun swap a b = {       "
      "        var t = *a;        "
      "        *a = *b;           "
      "        *b = t;            "
      "    };                     "
      "                           "
      "    fun main = {           "
      "        var a = 3;         "
      "        var b = 4;         "
      "                           "
      "        swap(&a, &b);      "
      "                           "
      "        var t = true;      "
      "        var f = false;     "
      "                           "
      "        swap(&t, &f);      "
      "    };                     ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(1)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}

TEST_CASE("vm:codgen:simple:mul", "[vm:codgen]") {
  types::Type::type_store.clear();
  char stream[] =
      "        fun mul a b = {                        "
      "           if a == 1 {                         "
      "              b                                "
      "           } else {                            "
      "              b + mul(a-1, b)                  "
      "           }                                   "
      "        };                                     "
      "                                               "
      "        fun main = {                           "
      "            mul(3, 5)                          "
      "        };                                     "
      "                                               ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(1)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}

///////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:struct", "[vm:codgen]") {
  types::Type::type_store.clear();
  char stream[] =
      "  type Str = struct {                            \n"
      "      count: Int,                                \n"
      "      ismodified: Bool,                          \n"
      "  };                                             \n"
      "                                                 \n"
      "  fun main = {                                   \n"
      "     of Str                                      \n"
      "     var instance = { .count = 4,                \n"
      "                      .ismodified = false, };    \n"
      "     instance.ismodified = true;                 \n"
      "     instance.ismodified                         \n"
      "  };                                             \n"
      "                                                 \n";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(1)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:struct:nested", "[vm:codgen]") {
  types::Type::type_store.clear();
  char stream[] =
      "                                         "
      "    type Inner = struct {                "
      "        f1: Int,                         "
      "        f2: Int,                         "
      "    };                                   "
      "                                         "
      "    type Str = struct {                  "
      "        int: Int,                        "
      "        inn: Inner,                      "
      "        bool: Bool,                      "
      "    };                                   "
      "                                         "
      "    fun main = {                         "
      "        of Str var i = {                 "
      "          .int = 4,                      "
      "          .inn = { .f1 = 0, .f2 = 1 },   "
      "          .bool = false,                 "
      "        };                               "
      "                                         "
      "        i.inn;                           "
      "        i.inn.f1 = i.int + i.inn.f2;     "
      "    };                                   "
      "                                         ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(2)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:array", "[vm:codgen]") {
  types::Type::type_store.clear();
  char stream[] =
      "        fun main = {                              "
      "           var vec = new [10] _;                  "
      "           vec[3] = 5;                            "
      "        };                                        ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(0)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:struct:tree", "[vm:codgen]") {
  types::Type::type_store.clear();
  char stream[] =
      "   type Tree a = struct {                     "
      "       left: *Tree(a),                           "
      "       right: *Tree(a),                          "
      "       value: a,                              "
      "   };                                         "
      "                                              "
      "   fun consTree val = {                       "
      "       var tree = new Tree(_);                "
      "       tree->left = unit ~> _;                "
      "       tree->right = unit ~> _;               "
      "       tree->value = val;                     "
      "       tree                                   "
      "   };                                         "
      "                                              "
      "   fun main = {                               "
      "       var tr1 = consTree(1);                 "
      "       of Tree(Int) var tree = {.value = 123,};"
      "       var tr2 = consTree(tree);             "
      "       tr2 = consTree(*tr1);             "
      //"       insertNewValue(tr1, consTree(5));      "
      //"       insertNewValue(tr1, consTree(4));      "
      //"       insertNewValue(tr1, consTree(3));      "
      //"       insertNewValue(tr1, consTree(2));      "
      "   };                                         ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(2)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:array:dynsize", "[vm:codgen]") {
  types::Type::type_store.clear();
  char stream[] =
      "    fun iterateVec vec size = {               "
      "      if size == 0 {                          "
      "        return;                               "
      "      };                                      "
      "                                              "
      "      *vec = (*vec) + 1;                      "
      "                                              "
      "      return iterateVec(vec + 1, size - 1);   "
      "    };                                        "
      "                                              "
      "    fun makeInt = 8;                          "
      "                                              "
      "    fun main = {                              "
      "       var vec = new [makeInt()] _;           "
      "                                              "
      "       vec[3] = 5;                            "
      "       vec[1] = 3;                            "
      "                                              "
      "       iterateVec(vec, makeInt());            "
      "    };                                        ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(2)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:assert", "[vm:codgen]") {
  types::Type::type_store.clear();
  // assert : Bool -> Unit
  char stream[] =
      "    fun main = {                              "
      "       assert(true);                          "
      "    };                                        ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  ast::elaboration::MarkIntrinsics mark;
  for (auto& r : result) {
    r = mark.Eval(r)->as<Statement>();
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  types::check::TemplateInstantiator inst(result.at(0)->as<FunDeclStatement>());
  auto [funs,_] = inst.Flush();

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  debugger.StepToTheEnd();
}
