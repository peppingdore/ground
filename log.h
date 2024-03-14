#pragma once

#include "base.h"
#include "format.h"
#include "file_path.h"

struct Logger;

enum class LogLevel: u32 {
	Error       = 0,
	Important   = 100,
	Warning     = Important,
	Verbose     = 200,
	Debug       = 300,
	Trace       = 400,
};

struct LogInfo {
	CodeLocation loc = caller_loc();
	LogLevel    level = LogLevel::Verbose;
};

using Logger_Proc = void (*) (Logger*, LogInfo, UnicodeString);

struct Logger {
	Allocator   allocator = c_allocator;
	Logger_Proc proc = NULL;
	LogLevel    pass_level = LogLevel::Trace;
};

void default_logger_proc(Logger* logger, LogInfo info, UnicodeString text) {
	auto unicode_path = make_string(info.loc.file).copy_unicode_string();
	auto base = path_basename(unicode_path);
	auto formatted = sprint("%:% %", base, info.loc.line, text);
	print(formatted);
	formatted.free();
	unicode_path.free();
}

Logger* make_default_logger() {
	auto logger = make<Logger>();
	logger->proc = default_logger_proc;
	return logger;
}

void void_logger_proc(Logger* logger, LogInfo info, UnicodeString text) {

}

Logger* make_void_logger() {
	auto logger = make<Logger>();
	logger->proc = void_logger_proc;
	return logger;
}

inline Logger*(*make_new_logger)() = make_default_logger;
inline thread_local Logger* __thread_logger = NULL;

inline Logger* set_thread_logger(Logger* logger) {
	swap(logger, __thread_logger);
	return logger;
}

inline Logger* get_logger() {
	if (!__thread_logger) {
		__thread_logger = make_new_logger();
	}
	return __thread_logger;
}


// Using CamelCase instead of snake_case here to avoid intersecting
//   with math log() function.

inline void LogAtLogger(Logger* logger, LogInfo info, UnicodeString text) {
	logger->proc(logger, info, text);
}

inline void LogAtLogger(Logger* logger, LogInfo info, auto... args) {
	auto str = sprint_unicode(logger->allocator, args...);
	LogAtLogger(logger, info, str);
}

inline void LogWithInfo(LogInfo info, auto... args) {
	LogAtLogger(get_logger(), info, args...);
}

inline void Log(CodeLocation loc, auto... args) {
	LogWithInfo({ .loc = loc, }, args...);
}

inline void LogTrace(CodeLocation loc, auto... args) {
	LogWithInfo({ .loc = loc, .level = LogLevel::Trace }, args...);
}

// We can't use loc = caller_loc() if we have parameter pack.
//   These macros help us with that situation.
#define Log(...)      Log(current_loc(), __VA_ARGS__)
#define LogTrace(...) LogTrace(current_loc(), __VA_ARGS__)
