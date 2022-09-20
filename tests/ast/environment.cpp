#include <ast/scope/environment.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Scoping", "[rt]") {
  Environment<int> global = Environment<int>::MakeGlobal();

  Environment<int>* interpreter_scope = &global;
  interpreter_scope->Declare("a", (3));
  CHECK(interpreter_scope->Get("a") == (3));

  {
    Environment<int>::ScopeGuard guard1{&interpreter_scope};

    // Shadow global a

    interpreter_scope->Declare("a", (5));
    CHECK(interpreter_scope->Get("a") == (5));

    {
      Environment<int>::ScopeGuard guard2{&interpreter_scope};

      // Assign to a from the first outer scope

      interpreter_scope->TryAssign("a", (9));
    }

    CHECK(interpreter_scope->Get("a") == (9));
  }

  CHECK(interpreter_scope->Get("a") == (3));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Shadowing", "[rt]") {
  Environment<int> global = Environment<int>::MakeGlobal();

  Environment<int>* interpreter_scope = &global;
  interpreter_scope->Declare("a", (3));

  {
    Environment<int>::ScopeGuard guard1{&interpreter_scope};
    CHECK(interpreter_scope->Get("a") == (3));

    // Shadow

    interpreter_scope->Declare("a", (9));
    CHECK(interpreter_scope->Get("a") == (9));
  }

  CHECK(interpreter_scope->Get("a") == (3));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Assign", "[rt]") {
  Environment<int> global = Environment<int>::MakeGlobal();

  Environment<int>* interpreter_scope = &global;
  interpreter_scope->Declare("a", (3));

  {
    Environment<int>::ScopeGuard guard1{&interpreter_scope};

    // Insert
    interpreter_scope->Declare("b", (4));

    // Assign
    interpreter_scope->TryAssign("a", (4));
  }

  CHECK(interpreter_scope->Get("a") == (4));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Get", "[rt]") {
  Environment<int> global = Environment<int>::MakeGlobal();

  Environment<int>* interpreter_scope = &global;
  interpreter_scope->Declare("a", (3));

  {
    Environment<int>::ScopeGuard guard1{&interpreter_scope};
    interpreter_scope->Declare("b", (4));

    {
      Environment<int>::ScopeGuard guard2{&interpreter_scope};
      interpreter_scope->Declare("c", (5));
      CHECK_NOTHROW(interpreter_scope->Get("a").value());
      CHECK_NOTHROW(interpreter_scope->Get("b").value());
      CHECK_NOTHROW(interpreter_scope->Get("c").value());
    }

    CHECK_NOTHROW(interpreter_scope->Get("a").value());
    CHECK_NOTHROW(interpreter_scope->Get("b").value());
    CHECK_THROWS(interpreter_scope->Get("c").value());
  }

  CHECK_NOTHROW(interpreter_scope->Get("a").value());
  CHECK_THROWS(interpreter_scope->Get("b").value());
  CHECK_THROWS(interpreter_scope->Get("c").value());
}

//////////////////////////////////////////////////////////////////////
