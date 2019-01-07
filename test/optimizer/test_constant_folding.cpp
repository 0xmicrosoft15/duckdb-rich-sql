// #include "catch.hpp"
// #include "common/helper.hpp"
// #include "expression_helper.hpp"
// #include "optimizer/expression_rules/list.hpp"
// #include "optimizer/rewriter.hpp"
// #include "parser/expression/list.hpp"

// #include <vector>

// using namespace duckdb;
// using namespace std;

// // ADD(42, 1) -> 43
// TEST_CASE("Constant folding does something", "[optimizer]") {
// 	BindContext context;
// 	vector<unique_ptr<Rule>> rules;
// 	rules.push_back(unique_ptr<Rule>(new ConstantFoldingRule()));
// 	auto rewriter = Rewriter(context, move(rules), MatchOrder::DEPTH_FIRST);

// 	auto left = make_unique<ConstantExpression>(Value::INTEGER(42));
// 	auto right = make_unique<ConstantExpression>(Value::INTEGER(1));

// 	auto root = make_unique_base<Expression, OperatorExpression>(ExpressionType::OPERATOR_ADD, TypeId::INTEGER,
// 	                                                             move(left), move(right));

// 	auto result = ApplyExprRule(rewriter, move(root));

// 	REQUIRE(result->type == ExpressionType::VALUE_CONSTANT);

// 	auto result_cast = reinterpret_cast<ConstantExpression *>(result.get());

// 	REQUIRE(result_cast->value.value_.integer == 43);
// 	REQUIRE(result_cast->children.size() == 0);
// }

// // ADD(ADD(42, 1), 10) -> 53
// TEST_CASE("Constant folding finishes in fixpoint", "[optimizer]") {
// 	BindContext context;

// 	vector<unique_ptr<Rule>> rules;
// 	rules.push_back(unique_ptr<Rule>(new ConstantFoldingRule()));
// 	auto rewriter = Rewriter(context, move(rules), MatchOrder::DEPTH_FIRST);

// 	auto ll = make_unique<ConstantExpression>(Value::INTEGER(42));
// 	auto lr = make_unique<ConstantExpression>(Value::INTEGER(1));

// 	unique_ptr<Expression> il =
// 	    make_unique<OperatorExpression>(ExpressionType::OPERATOR_ADD, TypeId::INTEGER, move(ll), move(lr));
// 	auto ir = make_unique<ConstantExpression>(Value::INTEGER(10));
// 	unique_ptr<Expression> root =
// 	    make_unique<OperatorExpression>(ExpressionType::OPERATOR_ADD, TypeId::INTEGER, move(il), move(ir));

// 	auto result = ApplyExprRule(rewriter, move(root));

// 	REQUIRE(result->type == ExpressionType::VALUE_CONSTANT);

// 	auto result_cast = reinterpret_cast<ConstantExpression *>(result.get());

// 	REQUIRE(result_cast->value.value_.integer == 53);
// 	REQUIRE(result_cast->children.size() == 0);
// }

// // MUL(42, SUB(10, 9)) -> 42
// TEST_CASE("Constant folding reduces complex expression", "[optimizer]") {
// 	BindContext context;

// 	vector<unique_ptr<Rule>> rules;
// 	rules.push_back(unique_ptr<Rule>(new ConstantFoldingRule()));
// 	auto rewriter = Rewriter(context, move(rules), MatchOrder::DEPTH_FIRST);

// 	auto ll = make_unique<ConstantExpression>(Value::INTEGER(10));
// 	auto lr = make_unique<ConstantExpression>(Value::INTEGER(9));

// 	unique_ptr<Expression> ir =
// 	    make_unique<OperatorExpression>(ExpressionType::OPERATOR_SUBTRACT, TypeId::INTEGER, move(ll), move(lr));
// 	auto il = make_unique<ConstantExpression>(Value::INTEGER(42));
// 	unique_ptr<Expression> root =
// 	    make_unique<OperatorExpression>(ExpressionType::OPERATOR_MULTIPLY, TypeId::INTEGER, move(il), move(ir));

// 	auto result = ApplyExprRule(rewriter, move(root));

// 	REQUIRE(result->type == ExpressionType::VALUE_CONSTANT);

// 	auto result_cast = reinterpret_cast<ConstantExpression *>(result.get());

// 	REQUIRE(result_cast->value.GetNumericValue() == 42);
// 	REQUIRE(result_cast->children.size() == 0);
// }
// //
// //// MUL(WHATEV, 0) -> 0
// // TEST_CASE("Constant folding handles unknown expressions left", "[optimizer]")
// // {
// //
// //	vector<unique_ptr<Rule>> rules;
// //	rules.push_back(unique_ptr<Rule>(new ConstantFoldingRule()));
// //	auto rewriter = Rewriter(move(rules), MatchOrder::DEPTH_FIRST);
// //
// //	auto lr = make_unique<ConstantExpression>(Value::INTEGER(0));
// //	auto ll = make_unique<ColumnRefExpression>("WHATEV");
// //	ll->return_type = TypeId::BIGINT;
// //
// //	unique_ptr<Expression> root = make_unique<OperatorExpression>(
// //	    ExpressionType::OPERATOR_MULTIPLY, TypeId::INTEGER, move(ll), move(lr));
// //
// //	auto result = ApplyExprRule(rewriter, move(root));
// //
// //	REQUIRE(result->type == ExpressionType::VALUE_CONSTANT);
// //
// //	auto result_cast = reinterpret_cast<ConstantExpression *>(result.get());
// //
// //	REQUIRE(result_cast->value.GetNumericValue() == 0);
// //	REQUIRE(result_cast->children.size() == 0);
// //}
// //
// //// ADD(0, WHATEV) -> WHATEV
// // TEST_CASE("Constant folding handles unknown expressions right",
// // "[optimizer]") {
// //
// //	vector<unique_ptr<Rule>> rules;
// //	rules.push_back(unique_ptr<Rule>(new ConstantFoldingRule()));
// //	auto rewriter = Rewriter(move(rules), MatchOrder::DEPTH_FIRST);
// //
// //	auto ll = make_unique<ConstantExpression>(Value::INTEGER(0));
// //	auto lr = make_unique<ColumnRefExpression>("WHATEV");
// //	lr->return_type = TypeId::BIGINT;
// //
// //	unique_ptr<Expression> root = make_unique<OperatorExpression>(
// //	    ExpressionType::OPERATOR_ADD, TypeId::INTEGER, move(ll), move(lr));
// //
// //	auto result = ApplyExprRule(rewriter, move(root));
// //
// //	REQUIRE(result->type == ExpressionType::COLUMN_REF);
// //	REQUIRE(result->children.size() == 0);
// //}
// //
// //// MUL(42, DIV(WHATEV, 0)) -> NULL
// // TEST_CASE("Constant folding handles NULL propagation", "[optimizer]") {
// //	vector<unique_ptr<Rule>> rules;
// //	rules.push_back(unique_ptr<Rule>(new ConstantFoldingRule()));
// //	auto rewriter = Rewriter(move(rules), MatchOrder::DEPTH_FIRST);
// //
// //	auto lr = make_unique<ConstantExpression>(Value::INTEGER(0));
// //	auto ll = make_unique<ColumnRefExpression>("WHATEV");
// //	ll->return_type = TypeId::BIGINT;
// //
// //	unique_ptr<Expression> ir = make_unique<OperatorExpression>(
// //	    ExpressionType::OPERATOR_DIVIDE, TypeId::INTEGER, move(ll), move(lr));
// //	auto il = make_unique<ConstantExpression>(Value::INTEGER(42));
// //
// //	unique_ptr<Expression> root = make_unique<OperatorExpression>(
// //	    ExpressionType::OPERATOR_MULTIPLY, TypeId::INTEGER, move(il), move(ir));
// //
// //	auto result = ApplyExprRule(rewriter, move(root));
// //
// //	REQUIRE(result->type == ExpressionType::VALUE_CONSTANT);
// //
// //	auto result_cast = reinterpret_cast<ConstantExpression *>(result.get());
// //
// //	REQUIRE(result_cast->value.is_null);
// //	REQUIRE(result_cast->children.size() == 0);
// //}

// //// CAST(42.0 AS INTEGER) -> 42
// TEST_CASE("Constant casting does something", "[optimizer]") {
// 	BindContext context;

// 	vector<unique_ptr<Rule>> rules;
// 	rules.push_back(unique_ptr<Rule>(new ConstantCastRule()));
// 	auto rewriter = Rewriter(context, move(rules), MatchOrder::DEPTH_FIRST);
// 	auto child = make_unique<ConstantExpression>(Value(42.0));
// 	unique_ptr<Expression> root = make_unique<CastExpression>(TypeId::INTEGER, move(child));

// 	auto result = ApplyExprRule(rewriter, move(root));
// 	REQUIRE(result->type == ExpressionType::VALUE_CONSTANT);
// 	REQUIRE(result->return_type == TypeId::INTEGER);

// 	auto result_cast = reinterpret_cast<ConstantExpression *>(result.get());

// 	REQUIRE(!result_cast->value.is_null);
// 	REQUIRE(result_cast->value.value_.integer == 42);

// 	REQUIRE(result_cast->children.size() == 0);
// }
