//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// optimizer/expression_rules/distributivity.hpp
//
// Author: Hannes Mühleisen & Mark Raasveldt
//
//===----------------------------------------------------------------------===//

#pragma once

#include "optimizer/rule.hpp"

namespace duckdb {

// (X AND B) OR (X AND C) = X AND (B OR C)
class DistributivityRule : public Rule {
  public:
	DistributivityRule();

	std::unique_ptr<Expression> Apply(Rewriter &rewriter, Expression &root,
	                                  std::vector<AbstractOperator> &bindings);
};

} // namespace duckdb
