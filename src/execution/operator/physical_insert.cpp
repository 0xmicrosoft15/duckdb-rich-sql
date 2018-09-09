#include "execution/operator/physical_insert.hpp"
#include "execution/expression_executor.hpp"

#include "storage/data_table.hpp"

using namespace duckdb;
using namespace std;

vector<TypeId> PhysicalInsert::GetTypes() { return {TypeId::INTEGER}; }

void PhysicalInsert::GetChunk(ClientContext &context, DataChunk &chunk, PhysicalOperatorState *state) {

	chunk.Reset();

	if (children.size() > 0) {
		// get the next chunk from the child
		children[0]->GetChunk(context, state->child_chunk, state->child_state.get());
		if (state->child_chunk.count == 0) {
			return;
		}
	} else {
		if (state->finished) {
			return;
		}
	}

	DataChunk insert_chunk;
	vector<TypeId> types;
	for (auto &column : table->columns) {
		types.push_back(column.type);
	}
	insert_chunk.Initialize(types);
	ExpressionExecutor executor(children.size() == 0 ? nullptr : state, context);

	for (size_t i = 0; i < value_list.size(); i++) {
		auto &expr = value_list[i];
		executor.Execute(expr.get(), insert_chunk.data[i]);
	}
	insert_chunk.count = insert_chunk.data[0].count;

	for (size_t i = 0; i < insert_chunk.column_count; i++) {
		if (insert_chunk.count != insert_chunk.data[i].count) {
			throw Exception("Insert count mismatch!");
		}
	}
	chunk.data[0].count = 1;
	chunk.data[0].SetValue(0, Value::INTEGER(insert_chunk.data[0].count));

	table->storage->Append(context.ActiveTransaction(), insert_chunk);

	chunk.count = 1;

	state->finished = true;

	chunk.Verify();
}

unique_ptr<PhysicalOperatorState>
PhysicalInsert::GetOperatorState(ExpressionExecutor *parent_executor) {
	return make_unique<PhysicalOperatorState>(nullptr, parent_executor);
}
