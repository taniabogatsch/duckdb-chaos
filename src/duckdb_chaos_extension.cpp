#define DUCKDB_EXTENSION_MAIN

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb_chaos_extension.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include <signal.h>

namespace duckdb {

enum class SignalType : uint8_t {
	SIGNAL_SIGSEGV = 0,
	SIGNAL_SIGABRT = 1,
	SIGNAL_SIGBUS = 2
};

static SignalType SignalTypeFromString(const string &signal_type_str) {
	if (StringUtil::CIEquals(signal_type_str, "SIGSEGV")) {
		return SignalType::SIGNAL_SIGSEGV;
	}
	if (StringUtil::CIEquals(signal_type_str, "SIGABRT")) {
		return SignalType::SIGNAL_SIGABRT;
	}
	if (StringUtil::CIEquals(signal_type_str, "SIGBUS")) {
		return SignalType::SIGNAL_SIGBUS;
	}
	throw InvalidInputException("Invalid signal type: %s. Valid signal types are: SIGSEGV, SIGABRT, and SIGBUS.", signal_type_str);
}

inline void DuckDBChaosExceptionFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &message_col = args.data[0];
	auto &exception_type_col = args.data[1];

	BinaryExecutor::Execute<string_t, string_t, string_t>(message_col, exception_type_col, result, args.size(), [&](string_t message, string_t exception_type_str) -> string_t {
		auto exception_type = EnumUtil::FromString<ExceptionType>(exception_type_str.GetString());
		throw Exception(exception_type, message.GetString());
	});
}

inline void DuckDBChaosSignalFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &signal_col = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(signal_col, result, args.size(), [&](string_t signal_type_str) -> string_t {
		auto signal_type = SignalTypeFromString(signal_type_str.GetString());
		switch (signal_type) {
		case SignalType::SIGNAL_SIGSEGV:
			raise(SIGSEGV);
		case SignalType::SIGNAL_SIGABRT:
			raise(SIGABRT);
		case SignalType::SIGNAL_SIGBUS:
			raise(SIGBUS);
		}
		throw InternalException("missing case for signal type: %s", signal_type_str.GetString());
	});
}

static void LoadInternal(DatabaseInstance &instance) {
	// Register a scalar function to throw an exception.
	ScalarFunction exception_fun("duckdb_chaos_exception", {LogicalType::VARCHAR, LogicalType::VARCHAR}, LogicalType::SQLNULL, DuckDBChaosExceptionFun);
	exception_fun.stability = FunctionStability::VOLATILE;
	BaseScalarFunction::SetReturnsError(exception_fun);
	ExtensionUtil::RegisterFunction(instance, exception_fun);

	// Register a scalar function to invoke a signal.
	ScalarFunction signal_fun("duckdb_chaos_signal", {LogicalType::VARCHAR}, LogicalType::SQLNULL, DuckDBChaosSignalFun);
	signal_fun.stability = FunctionStability::VOLATILE;
	BaseScalarFunction::SetReturnsError(signal_fun);
	ExtensionUtil::RegisterFunction(instance, signal_fun);
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
