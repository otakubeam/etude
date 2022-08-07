#include <ast/visitors/evaluator.hpp>

#include <rt/functions/intrinsic_print.hpp>

//////////////////////////////////////////////////////////////////////

Evaluator::Evaluator()
    : struct_decls_{Environment<StructDeclStatement*>::MakeGlobal()} {
  env_->Declare("print", new rt::Print{});
}

Evaluator::~Evaluator() = default;

//////////////////////////////////////////////////////////////////////

void Evaluator::VisitExpression(Expression* /* node */) {
  FMT_ASSERT(false, "\nVisiting bare expression\n");
}

//////////////////////////////////////////////////////////////////////

void Evaluator::VisitComparison(ComparisonExpression* node) {
  auto lhs = Eval(node->left_);
  auto rhs = Eval(node->right_);

  switch (node->operator_.type) {
    case lex::TokenType::EQUALS:
      return_value = {rt::PrimitiveObject{lhs == rhs}};
      break;

    case lex::TokenType::LT:
      // TODO: comparions of primitive types
      // return_value = {PrimitiveType{lhs < rhs}};
      std::abort();
      break;

    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

void Evaluator::VisitBinary(BinaryExpression* node) {
  auto lhs = Eval(node->left_);
  auto rhs = Eval(node->right_);

  switch (node->operator_.type) {
    case lex::TokenType::PLUS:
      return_value = Plus(lhs, rhs);
      break;

    case lex::TokenType::MINUS:
      return_value = Minus(lhs, rhs);
      break;

    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

void Evaluator::VisitUnary(UnaryExpression* node) {
  auto val = Eval(node->operand_);

  switch (node->operator_.type) {
    case lex::TokenType::NOT:
      return_value = Bang(val);
      break;

    case lex::TokenType::MINUS:
      return_value = Negate(val);
      break;

    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

void Evaluator::VisitFnCall(FnCallExpression* node) {
  std::vector<rt::SBObject> args;

  for (auto expr : node->arguments_) {
    args.push_back(Eval(expr));
  }

  auto fn_object = std::get<rt::IFunction*>(  //
      env_->Get(                              //
              node->fn_name_.GetName())
          .value());

  auto ret = rt::SBObject{};

  try {
    ret = fn_object->Compute(this, args);
  } catch (ReturnedValue val) {
    ret = val.value;
  }

  return_value = ret;
}

//////////////////////////////////////////////////////////////////////

void Evaluator::VisitLiteral(LiteralExpression* lit) {
  switch (lit->token_.type) {
    case lex::TokenType::IDENTIFIER:
      std::abort();

    case lex::TokenType::TRUE:
      return_value = {rt::PrimitiveObject{true}};
      break;

    case lex::TokenType::FALSE:
      return_value = {rt::PrimitiveObject{false}};
      break;

    default:
      auto val = lit->token_.sem_info;
      return_value = {rt::FromSemInfo(val)};
  }
}

//////////////////////////////////////////////////////////////////////

// In this case we promote it to rvalue
// In assignment we would want to extract the name
void Evaluator::VisitLvalue(VarAccessExpression* lit) {
  // Asserts that we are an IDENTIFIER
  auto name = lit->name_.GetName();

  // Name should be bound during type-cheking
  return_value = env_->Get(name).value();
}

//////////////////////////////////////////////////////////////////////
