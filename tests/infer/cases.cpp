#include <ast/scope/context_builder.hpp>

#include <types/instantiate/instantiator.hpp>
#include <types/check/algorithm_w.hpp>

#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

using types::Type;
using types::TypeTag;

//////////////////////////////////////////////////////////////////////

Type a = Type{.id = 1, .tag = TypeTag::TY_PARAMETER};
Type ptr_a = Type{.tag = types::TypeTag::TY_PTR, .as_ptr = {.underlying = &a}};

Type ptr_int = Type{
    .tag = types::TypeTag::TY_PTR,
    .as_ptr = {.underlying = &types::builtin_int},
};

Type ptr_bool = Type{
    .tag = types::TypeTag::TY_PTR,
    .as_ptr = {.underlying = &types::builtin_bool},
};

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:simple", "[infer]") {
  char stream[] = "fun f x = { x + 1 };";
  std::stringstream source{stream};
  lex::Lexer l{source};
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

  Type t1 = Type{.id = 1, .tag = TypeTag::TY_PARAMETER};

  Type t3 = Type{.tag = TypeTag::TY_FUN,
                 .as_fun = {
                     .param_pack = {&t1},
                     .result_type = &t1,
                 }};

  CHECK(TypesEquivalent(&t3, global_context.RetrieveFromChild("f")->GetType()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:pointer", "[infer]") {
  char stream[] = "fun f p = { *p + 1 };";
  std::stringstream source{stream};
  lex::Lexer l{source};
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

  Type t3 = Type{.tag = TypeTag::TY_FUN,
                 .as_fun = {
                     .param_pack = {&ptr_a},
                     .result_type = &a,
                 }};
  CHECK(types::TypesEquivalent(
      &t3, global_context.RetrieveFromChild("f")->GetType()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:pointer-II", "[infer]") {
  char stream[] = "fun f p = { *(p + *p) };";
  std::stringstream source{stream};
  lex::Lexer l{source};
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

  Type t3 = Type{.tag = TypeTag::TY_FUN,
                 .as_fun = {
                     .param_pack = {&ptr_int},
                     .result_type = &types::builtin_int,
                 }};
  CHECK(types::TypesEquivalent(
      &t3, global_context.RetrieveFromChild("f")->GetType()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:recursive:simple", "[infer]") {
  char stream[] = "fun fib n = if n == 0 1 else fib(n - 1) + fib(n - 2);";
  std::stringstream source{stream};
  lex::Lexer l{source};
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

  Type t3 = Type{.tag = TypeTag::TY_FUN,
                 .as_fun = {.param_pack = {&types::builtin_int},
                            .result_type = &types::builtin_int}};
  CHECK(types::TypesEquivalent(
      &t3, global_context.RetrieveFromChild("fib")->GetType()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:poly:id", "[infer]") {
  char stream[] =
      "    fun id x = x;      "
      "                       "
      "    fun main = {       "
      "        id(3);         "
      "        id(true);      "
      "        var t = id;    "
      "        var k = t(1);  "
      "    };                 ";
  std::stringstream source{stream};
  lex::Lexer l{source};
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

  Type t3 = Type{.tag = TypeTag::TY_FUN,
                 .as_fun = {.param_pack = {&types::builtin_int},
                            .result_type = &types::builtin_int}};
  CHECK(types::TypesEquivalent(
      &t3, global_context.RetrieveFromChild("t")->GetType()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:poly:inst", "[infer]") {
  char stream[] =
      "    fun id x = x;      "
      "                       "
      "    fun main = {       "
      "        id(3);         "
      "        id(true);      "
      "    };                 ";
  std::stringstream source{stream};
  lex::Lexer l{source};
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

  types::check::TemplateInstantiator inst(result[1]->as<FunDeclStatement>());
  inst.Flush();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:poly:swap", "[infer]") {
  char stream[] =
      "    fun swap a b = {       "
      "       var t = *a;         "
      "       *a = *b;            "
      "       *b = t;             "
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
  lex::Lexer l{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  global_context.Print();

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  global_context.Print();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:type", "[infer]") {
  char stream[] =
      "    type Vec T = struct {       \n"
      "       size: Int,               \n"
      "       data: *T,                \n"
      "    };                          \n"
      "                                \n"
      "    of *Vec(Int) -> Unit        \n"
      "    fun takeVecInt v = {};      \n"
      "                                \n"
      "    fun main = {                \n"
      "       type R = Vec(_);         \n"
      "       var t = (new _) ~> *R;   \n"
      "       takeVecInt(t);           \n"
      "    };                          \n"
      "                                \n";
  std::stringstream source{stream};
  lex::Lexer l{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  global_context.Print();

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  global_context.Print();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:type:recursive", "[infer]") {
  char stream[] =
      "                       "
      " type Tree = struct {  "
      "    tag: Int,          "
      "    left: *Tree,       "
      " };                    "
      "                       "
      " fun main = {          "
      "   var a = new Tree;   "
      " };                    "
      "                       ";
  std::stringstream source{stream};
  lex::Lexer l{source};
  Parser p{l};
  auto result = p.ParseModule();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  global_context.Print();

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  global_context.Print();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:equivalence:1", "[infer]") {
  using namespace types;

  Type t1 = Type{.id = 1, .tag = TypeTag::TY_PARAMETER};
  Type t2 = Type{.id = 2, .tag = TypeTag::TY_PARAMETER};

  CHECK(TypesEquivalent(&t1, &t2));

  Type t3 = Type{.tag = TypeTag::TY_FUN,
                 .as_fun = {.param_pack = {&t1, &t1}, .result_type = &t2}};

  Type t4 = Type{.tag = TypeTag::TY_FUN,
                 .as_fun = {.param_pack = {&t2, &t2}, .result_type = &t1}};

  CHECK(TypesEquivalent(&t3, &t4));

  lex::Token t{lex::TokenType::IDENTIFIER, {}, ""};

  auto t5 = Type{.tag = types::TypeTag::TY_APP,
                 .as_tyapp = {.name = t, .param_pack = {&t4}}};
  auto t6 = Type{.tag = types::TypeTag::TY_APP,
                 .as_tyapp = {.name = t, .param_pack = {&t3}}};

  CHECK(TypesEquivalent(&t5, &t6));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:equivalence:2", "[infer]") {
  using namespace types;
}

//////////////////////////////////////////////////////////////////////
