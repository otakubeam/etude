#include <ast/visitors/evaluator.hpp>

#include <rt/intrinsic.hpp>

//////////////////////////////////////////////////////////////////////

Evaluator::Evaluator() {
  env_->Declare("print", new Print{});
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
      return_value = {PrimitiveType{lhs == rhs}};
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
  std::vector<SBObject> args;

  for (auto expr : node->arguments_) {
    args.push_back(Eval(expr));
  }

  auto fn_object = std::get<IFunction*>(  //
      env_->Get(                          //
              node->fn_name_.GetName())
          .value());

  auto ret = SBObject{};

  try {
    fn_object->Compute(this, args);
  } catch (ReturnedValue val) {
    ret = val.value;
  }

  return_value = ret;
}

//////////////////////////////////////////////////////////////////////

void Evaluator::VisitLiteral(LiteralExpression* lit) {
  switch (lit->token_.type) {
    case lex::TokenType::IDENTIFIER: {
      auto name = std::get<std::string>(lit->token_.sem_info);

      // TODO: think better about error handling
      return_value = env_->Get(name).value();
      //                            ----------

      break;
    }

    case lex::TokenType::TRUE:
      return_value = {PrimitiveType{true}};
      break;

    case lex::TokenType::FALSE:
      return_value = {PrimitiveType{false}};
      break;

    default:
      auto val = lit->token_.sem_info;
      return_value = {FromSemInfo(val)};
  }
}

//////////////////////////////////////////////////////////////////////
