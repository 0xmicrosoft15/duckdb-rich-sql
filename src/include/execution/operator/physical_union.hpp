//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// execution/operator/physical_union.hpp
//
// Author: Hannes Mühleisen
//
//===----------------------------------------------------------------------===//

#pragma once

#include "execution/physical_operator.hpp"

namespace duckdb {
class PhysicalUnion : public PhysicalOperator {
  public:
	PhysicalUnion(std::unique_ptr<PhysicalOperator> top,
	              std::unique_ptr<PhysicalOperator> bottom);

	std::vector<std::string> GetNames() override;
	std::vector<TypeId> GetTypes() override;

	void _GetChunk(ClientContext &context, DataChunk &chunk,
	               PhysicalOperatorState *state) override;
	std::unique_ptr<PhysicalOperatorState>
	GetOperatorState(ExpressionExecutor *parent_executor) override;
};

class PhysicalUnionOperatorState : public PhysicalOperatorState {
  public:
	PhysicalUnionOperatorState(ExpressionExecutor *parent_executor)
	    : PhysicalOperatorState(nullptr, parent_executor), top_done(false) {
	}
	std::unique_ptr<PhysicalOperatorState> top_state;
	std::unique_ptr<PhysicalOperatorState> bottom_state;
	bool top_done = false;
};

}; // namespace duckdb
