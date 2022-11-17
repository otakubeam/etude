#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>

#include <types/instantiate/instantiator.hpp>
#include <types/check/algorithm_w.hpp>

#include <qbe/ir_emitter.hpp>
#include <parse/parser.hpp>
#include <lex/lexer.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("qbe:simple", "[qbe]") {
  types::Type::type_store.clear();
  // assert : Bool -> Unit
  char stream[] =
      "    fun main = {                              "
      "       var t = 1337;                          "
      "       return 123;                            "
      "    t};                                       ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseUnit();

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
  auto funs = inst.Flush();

  qbe::IrEmitter ir;
  for (auto& fun : funs) {
    fun->Accept(&ir);
  }
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("qbe:if", "[qbe]") {
  types::Type::type_store.clear();
  char stream[] =
      "    fun main = {                              "
      "       var t = true;                          "
      "       if t { 123 } else { 200 }              "
      "     };                                       ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseUnit();

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
  auto funs = inst.Flush();

  qbe::IrEmitter ir;
  for (auto& fun : funs) {
    fun->Accept(&ir);
  }
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("qbe:struct", "[qbe]") {
  types::Type::type_store.clear();
  char stream[] =
      "    type Test = struct { x: Int, y: Int, };   "
      "                                              "
      "    fun main = {                              "
      "       of Test                                "
      "       var t = { .x = 14, .y = 15 };          "
      "       1                                      "
      "     };                                       "
      "                                              ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseUnit();

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

  types::check::TemplateInstantiator inst(result.at(1)->as<FunDeclStatement>());
  auto funs = inst.Flush();

  qbe::IrEmitter ir;
  for (auto& fun : funs) {
    fun->Accept(&ir);
  }
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("qbe:call", "[qbe]") {
  types::Type::type_store.clear();
  char stream[] =
      "    of Int -> Int fun id x = x;               "
      "                                              "
      "    fun main = {                              "
      "       id(123)                               "
      "     };                                       "
      "                                              ";
  std::stringstream source{stream};
  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseUnit();

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

  types::check::TemplateInstantiator inst(result.at(1)->as<FunDeclStatement>());
  auto funs = inst.Flush();

  qbe::IrEmitter ir;
  for (auto& fun : funs) {
    fun->Accept(&ir);
  }
}

//////////////////////////////////////////////////////////////////////
