//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// optimizer/rewriter.hpp
//
// Author: Hannes Mühleisen & Mark Raasveldt
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>

#include "optimizer/rule.hpp"

namespace duckdb {

class Rewriter {
  public:
	MatchOrder match_order;
	std::vector<std::unique_ptr<Rule>> rules;

	Rewriter() {}
	Rewriter(std::vector<std::unique_ptr<Rule>> rules, MatchOrder match_order)
	    : match_order(match_order), rules(std::move(rules)) {}

	std::unique_ptr<LogicalOperator>
	ApplyRules(std::unique_ptr<LogicalOperator> root);

	static bool MatchOperands(AbstractRuleNode *node, AbstractOperator rel,
	                          std::vector<AbstractOperator> &bindings);
};

} // namespace duckdb
