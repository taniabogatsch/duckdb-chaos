#define DUCKDB_EXTENSION_MAIN

#include "duckdb_chaos_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

namespace duckdb {

inline void DuckdbChaosScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "DuckdbChaos " + name.GetString() + " 🐥");
	});
}

static void LoadInternal(DatabaseInstance &instance) {
	// Register a scalar function
	auto duckdb_chaos_scalar_function = ScalarFunction("duckdb_chaos", {LogicalType::VARCHAR}, LogicalType::VARCHAR, DuckdbChaosScalarFun);
	ExtensionUtil::RegisterFunction(instance, duckdb_chaos_scalar_function);
}

void DuckdbChaosExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string DuckdbChaosExtension::Name() {
	return "duckdb_chaos";
}

std::string DuckdbChaosExtension::Version() const {
#ifdef EXT_VERSION_DUCKDB_CHAOS
	return EXT_VERSION_DUCKDB_CHAOS;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void duckdb_chaos_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::DuckdbChaosExtension>();
}

DUCKDB_EXTENSION_API const char *duckdb_chaos_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
