//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// execution/column_binding_resolver.hpp
//
// Author: Mark Raasveldt
//
//===----------------------------------------------------------------------===//

#pragma once

#include "planner/logical_operator.hpp"
#include "planner/logical_operator_visitor.hpp"

#include "duckdb.hpp"

namespace duckdb {

struct BoundTable {
	size_t table_index;
	size_t column_count;
	size_t column_offset;
};

//! The ColumnBindingResolver resolves ColumnBindings into base tables
//! (table_index, column_index) into physical indices into the DataChunks that
//! are used within the execution engine
class ColumnBindingResolver : public LogicalOperatorVisitor {
  public:
	ColumnBindingResolver() : current_depth(0) {
	}
	using LogicalOperatorVisitor::Visit;
	void Visit(LogicalCreateIndex &op);
	void Visit(LogicalUnion &op);
	void Visit(LogicalExcept &op);
	void Visit(LogicalIntersect &op);
	void Visit(LogicalCrossProduct &op);
	void Visit(LogicalGet &op);
	void Visit(LogicalJoin &op);
	void Visit(LogicalSubquery &op);
	void Visit(LogicalTableFunction &op);

	std::unique_ptr<Expression> Visit(ColumnRefExpression &expr);
	std::unique_ptr<Expression> Visit(SubqueryExpression &expr);

	std::vector<BoundTable> bound_tables;
	size_t current_depth;
	//! Append a list of tables to the current set of bound tables
	void AppendTables(std::vector<BoundTable> &right_tables);
};
} // namespace duckdb
