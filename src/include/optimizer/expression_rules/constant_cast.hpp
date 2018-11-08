//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// optimizer/expression_rules/constant_cast.hpp
//
// Author: Hannes Mühleisen & Mark Raasveldt
//
//===----------------------------------------------------------------------===//

#pragma once

#include "optimizer/rule.hpp"

namespace duckdb {

class ConstantCastRule : public Rule {
  public:
	ConstantCastRule();

	std::unique_ptr<Expression> Apply(Rewriter &rewriter, Expression &root,
	                                  std::vector<AbstractOperator> &bindings,
	                                  bool &fixed_point);
};

} // namespace duckdb
