#include "execution/operator/physical_insert.hpp"
#include "execution/expression_executor.hpp"

#include "storage/data_table.hpp"

using namespace duckdb;
using namespace std;

vector<TypeId> PhysicalInsert::GetTypes() { return {TypeId::BIGINT}; }

void PhysicalInsert::_GetChunk(ClientContext &context, DataChunk &chunk,
                               PhysicalOperatorState *state) {

	chunk.Reset();

	if (children.size() > 0) {
		// get the next chunk from the child
		children[0]->GetChunk(context, state->child_chunk,
		                      state->child_state.get());
		if (state->child_chunk.count == 0) {
			return;
		}
	} else {
		if (state->finished) {
			return;
		}
	}

	DataChunk insert_chunk, temp_chunk;
	vector<TypeId> types;
	for (auto &column : table->columns) {
		types.push_back(column.type);
	}
	insert_chunk.Initialize(types);
	temp_chunk.Initialize(types);
	ExpressionExecutor executor(children.size() == 0 ? nullptr : state,
	                            context);

	int64_t insert_count = 0;
	for (auto &list : insert_values) {
		for (size_t i = 0; i < list.size(); i++) {
			auto &expr = list[i];
			executor.Execute(expr.get(), temp_chunk.data[i]);
			assert(temp_chunk.data[i].count == 1);
			insert_chunk.data[i].Append(temp_chunk.data[i]);
		}
		insert_chunk.count++;
		if (insert_chunk.count == STANDARD_VECTOR_SIZE) {
			table->storage->Append(context.ActiveTransaction(), insert_chunk);
			insert_count += insert_chunk.count;
			insert_chunk.Reset();
		}
	}
	if (insert_chunk.count > 0) {
		table->storage->Append(context.ActiveTransaction(), insert_chunk);
		insert_count += insert_chunk.count;
	}

	chunk.data[0].count = 1;
	chunk.data[0].SetValue(0, Value::BIGINT(insert_count));
	chunk.count = 1;

	state->finished = true;
}

unique_ptr<PhysicalOperatorState>
PhysicalInsert::GetOperatorState(ExpressionExecutor *parent_executor) {
	return make_unique<PhysicalOperatorState>(nullptr, parent_executor);
}
