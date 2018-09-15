//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// planner/operator/logical_insert.hpp
//
// Author: Hannes Mühleisen & Mark Raasveldt
//
//===----------------------------------------------------------------------===//

#pragma once

#include "planner/logical_operator.hpp"

namespace duckdb {

//! LogicalInsert represents an insertion of data into a base table
class LogicalInsert : public LogicalOperator {
  public:
	LogicalInsert(TableCatalogEntry *table)
	    : LogicalOperator(LogicalOperatorType::INSERT), table(table) {}

	std::vector<std::vector<std::unique_ptr<AbstractExpression>>> insert_values;
	std::vector<int> column_index_map;

	virtual void Accept(LogicalOperatorVisitor *v) override { v->Visit(*this); }

	//! The base table to insert into
	TableCatalogEntry *table;
};
} // namespace duckdb
