#pragma once

#include "grd_base.h"
#include "grd_format.h"
#include "grd_file_path.h"

struct GrdLogger;

enum class GrdLogLevel: u32 {
	Error       = 0,
	Important   = 100,
	Warning     = Important,
	Verbose     = 200,
	Debug       = 300,
	Trace       = 400,
};

struct GrdLogInfo {
	GrdCodeLoc  loc = grd_caller_loc();
	GrdLogLevel level = GrdLogLevel::Verbose;
};

using GrdLogger_Proc = void (*) (GrdLogger*, GrdLogInfo, GrdUnicodeString);

struct GrdLogger {
	GrdAllocator   allocator = c_allocator;
	GrdLogger_Proc proc = NULL;
	GrdLogLevel    pass_level = GrdLogLevel::Trace;
};

void grd_default_logger_proc(GrdLogger* logger, GrdLogInfo info, GrdUnicodeString text) {
	auto unicode_path = grd_copy_unicode_string(grd_make_string(info.loc.file));
	auto base = grd_path_basename(unicode_path);
	auto formatted = grd_sprint("%:% %", base, info.loc.line, text);
	grd_println(formatted);
	formatted.free();
	unicode_path.free();
}

GrdLogger* grd_make_default_logger() {
	auto logger = grd_make<GrdLogger>();
	logger->proc = grd_default_logger_proc;
	return logger;
}

void grd_void_logger_proc(GrdLogger* logger, GrdLogInfo info, GrdUnicodeString text) {

}

GrdLogger* grd_make_void_logger() {
	auto logger = grd_make<GrdLogger>();
	logger->proc = grd_void_logger_proc;
	return logger;
}

inline GrdLogger*(*grd_make_new_logger)() = grd_make_default_logger;
inline thread_local GrdLogger* __grd_thread_logger = NULL;

inline GrdLogger* set_thread_logger(GrdLogger* logger) {
	grd_swap(logger, __grd_thread_logger);
	return logger;
}

inline GrdLogger* grd_get_logger() {
	if (!__grd_thread_logger) {
		__grd_thread_logger = grd_make_new_logger();
	}
	return __grd_thread_logger;
}


// Using CamelCase instead of snake_case here to avoid intersecting
//   with math log() function.

inline void GrdLogAtLogger(GrdLogger* logger, GrdLogInfo info, GrdUnicodeString text) {
	if (info.level > logger->pass_level) {
		return;
	}
	logger->proc(logger, info, text);
}

inline void GrdLogAtLogger(GrdLogger* logger, GrdLogInfo info, auto... args) {
	auto str = grd_sprint_unicode(logger->allocator, args...);
	GrdLogAtLogger(logger, info, (GrdUnicodeString) str);
	str.free();
}

inline void GrdLogWithInfo(GrdLogInfo info, auto... args) {
	GrdLogAtLogger(grd_get_logger(), info, args...);
}

inline void GrdLog(GrdCodeLoc loc, auto... args) {
	GrdLogWithInfo({ .loc = loc, }, args...);
}

inline void GrdLogTrace(GrdCodeLoc loc, auto... args) {
	GrdLogWithInfo({ .loc = loc, .level = GrdLogLevel::Trace }, args...);
}

// We can't use loc = grd_caller_loc() if we have parameter pack.
//   These macros help us with that situation.
#define GrdLog(...)      GrdLog(grd_current_loc(), __VA_ARGS__)
#define GrdLogTrace(...) GrdLogTrace(grd_current_loc(), __VA_ARGS__)
