//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// execution/operator/join/physical_cross_product.hpp
//
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "execution/physical_operator.hpp"

namespace duckdb {
//! PhysicalCrossProduct represents a cross product between two tables
class PhysicalCrossProduct : public PhysicalOperator {
  public:
	PhysicalCrossProduct(std::unique_ptr<PhysicalOperator> left,
	                     std::unique_ptr<PhysicalOperator> right);

	std::vector<std::string> GetNames() override;
	std::vector<TypeId> GetTypes() override;

	void _GetChunk(ClientContext &context, DataChunk &chunk,
	               PhysicalOperatorState *state) override;

	std::unique_ptr<PhysicalOperatorState>
	GetOperatorState(ExpressionExecutor *parent_executor) override;
};

class PhysicalCrossProductOperatorState : public PhysicalOperatorState {
  public:
	PhysicalCrossProductOperatorState(PhysicalOperator *left,
	                                  PhysicalOperator *right,
	                                  ExpressionExecutor *parent_executor)
	    : PhysicalOperatorState(left, parent_executor), left_position(0) {
		assert(left && right);
	}

	size_t left_position;
	DataChunk right_chunk;
	std::unique_ptr<PhysicalOperatorState> right_state;
};
} // namespace duckdb
