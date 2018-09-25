
#include "compare_result.hpp"

using namespace std;

namespace duckdb {

bool CHECK_COLUMN(unique_ptr<duckdb::DuckDBResult> &result,
                         size_t column_number,
                         vector<duckdb::Value> values) {
	if (!result->GetSuccess()) {
		fprintf(stderr, "Query failed with message: %s\n",
		        result->GetErrorMessage().c_str());
		//FAIL(result->GetErrorMessage().c_str());
		return false;
	}
	if (values.size() == 0) {
		if (result->size() != 0) {
			result->Print();
			//FAIL("Data size does not match value size!");
			return false;
		} else {
			return true;
		}
	}
	if (result->size() == 0) {
		//FAIL("Data size does not match value size!");
		return false;
	}
	if (column_number >= result->column_count()) {
		//FAIL("Column number out of range of result!");
		return false;
	}
	size_t chunk_index = 0;
	for (size_t i = 0; i < values.size();) {
		if (chunk_index > result->size()) {
			// ran out of chunks
			return false;
			//FAIL("Data size does not match value size!");
		}
		// check this vector
		auto &vector =
		    result->collection.chunks[chunk_index]->data[column_number];
		if (i + vector.count > values.size()) {
			vector.Print();
			// too many values in this vector
			//FAIL("Too many values in result!");
			return false;
		}
		for (size_t j = 0; j < vector.count; j++) {
			// NULL <> NULL, hence special handling
			if (vector.GetValue(j).is_null && values[i + j].is_null) {
				continue;
			}
			if (!Value::Equals(vector.GetValue(j), values[i + j])) {
				// FAIL("Incorrect result! Got " + vector.GetValue(j).ToString() +
				//      " but expected " + values[i + j].ToString());
				return false;
			}
		}
		chunk_index++;
		i += vector.count;
	}
	return true;
}

string compare_csv(unique_ptr<duckdb::DuckDBResult> &result,
                               string csv, bool header) {
	if (!result->GetSuccess()) {
		fprintf(stderr, "Query failed with message: %s\n",
		        result->GetErrorMessage().c_str());
		return result->GetErrorMessage().c_str();
	}
	string error;
	if (!compare_result(csv, result->collection, header, error)) {
		return error;
	}
	return "";
}


bool parse_datachunk(string csv, DataChunk &result,
                            bool has_header) {
	istringstream f(csv);
	string line;

	if (has_header) {
		auto split = StringUtil::Split(line, '|');
		if (split.size() != result.column_count) {
			// column length is different
			return false;
		}
	}
	size_t row = 0;
	while (getline(f, line)) {
		auto split = StringUtil::Split(line, '|');
		if (line.back() == '|') {
			split.push_back("");
		}
		if (split.size() != result.column_count) {
			// column length is different
			return false;
		}
		if (row >= STANDARD_VECTOR_SIZE) {
			// chunk is full, but parsing so far was successful
			// return the partially parsed chunk
			// this function is used for printing, and we probably don't want to
			// print >1000 rows anyway
			return true;
		}
		// now compare the values
		for (size_t i = 0; i < split.size(); i++) {
			// first create a string value
			Value value(split[i]);
			value.is_null = split[i].empty();

			// cast to the type of the column
			try {
				value = value.CastAs(result.data[i].type);
			} catch (...) {
				return false;
			}
			// now perform a comparison
			result.data[i].count++;
			result.data[i].SetValue(row, value);
		}
		result.count++;
		row++;
	}
	return true;
}

//! Compares the result of a pipe-delimited CSV with the given DataChunk
//! Returns true if they are equal, and stores an error_message otherwise
bool compare_result(string csv, ChunkCollection &collection,
                           bool has_header, string &error_message) {
	auto types = collection.types;
	DataChunk correct_result;

	istringstream f(csv);
	string line;
	size_t row = 0;
	/// read and parse the header line
	if (has_header) {
		getline(f, line);
		// check if the column length matches
		auto split = StringUtil::Split(line, '|');
		if (line.back() == '|') {
			split.push_back("");
		}
		if (split.size() != types.size()) {
			// column length is different
			goto incorrect;
		}
	}
	// now compare the actual data
	while (getline(f, line)) {
		if (collection.count <= row) {
			goto incorrect;
		}
		auto split = StringUtil::Split(line, '|');
		if (line.back() == '|') {
			split.push_back("");
		}
		if (split.size() != types.size()) {
			// column length is different
			goto incorrect;
		}
		// now compare the values
		for (size_t i = 0; i < split.size(); i++) {
			// first create a string value
			Value value(split[i]);
			value.is_null = split[i].empty();

			// cast to the type of the column
			try {
				value = value.CastAs(types[i]);
			} catch (...) {
				goto incorrect;
			}
			// now perform a comparison
			Value result_value = collection.GetValue(i, row);
			if (result_value.is_null && value.is_null) {
				// NULL = NULL in checking code
				continue;
			}
			if (value.type == TypeId::DECIMAL) {
				// round to two decimals
				auto left = StringUtil::Format("%.2f", value.value_.decimal);
				auto right =
				    StringUtil::Format("%.2f", result_value.value_.decimal);
				if (left != right) {
					double ldecimal = value.value_.decimal;
					double rdecimal = result_value.value_.decimal;
					if (ldecimal < 0.999 * rdecimal ||
					    ldecimal > 1.001 * rdecimal) {
						goto incorrect;
					}
				}
			} else {
				if (!Value::Equals(value, result_value)) {
					goto incorrect;
				}
			}
		}
		row++;
	}
	if (collection.count != row) {
		goto incorrect;
	}
	return true;
incorrect:
	correct_result.Initialize(types);
	if (!parse_datachunk(csv, correct_result, has_header)) {
		error_message = "Incorrect answer for query!\nProvided answer:\n" +
		                collection.ToString() +
		                "\nExpected answer [could not parse]:\n" +
		                string(csv) + "\n";
	} else {
		error_message = "Incorrect answer for query!\nProvided answer:\n" +
		                collection.ToString() + "\nExpected answer:\n" +
		                correct_result.ToString() + "\n";
	}
	return false;
}
}
