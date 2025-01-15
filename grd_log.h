#pragma once

#include "grd_base.h"
#include "grd_format.h"
#include "grd_file_path.h"
#include <ctype.h>

struct GrdLogger;

enum class GrdLogLevel: u32 {
	None        = 0,
	Error       = 50,
	Important   = 100,
	Warning     = Important,
	Verbose     = 200,
	Debug       = 300,
	Trace       = 400,
	All         = 1000000,
};
GRD_REFLECT(GrdLogLevel) {
	GRD_ENUM_VALUE(None);
	GRD_ENUM_VALUE(Error);
	GRD_ENUM_VALUE(Important);
	GRD_ENUM_VALUE(Warning);
	GRD_ENUM_VALUE(Verbose);
	GRD_ENUM_VALUE(Debug);
	GRD_ENUM_VALUE(Trace);
	GRD_ENUM_VALUE(All);
}

struct GrdLogInfo {
	GrdCodeLoc  loc    = grd_caller_loc();
	GrdLogLevel level  = GrdLogLevel::Verbose;
	s64         indent = 0;

	GRD_REFLECT(GrdLogInfo) {
		GRD_MEMBER(loc);
		GRD_MEMBER(level);
		GRD_MEMBER(indent);
	}
};

using GrdLoggerProc = void (*) (GrdLogger*, GrdLogInfo, GrdUnicodeString);

struct GrdLogger {
	GrdAllocator   allocator = c_allocator;
	GrdLoggerProc  proc = NULL;
	GrdLogLevel    pass_level = GrdLogLevel::Warning;

	GRD_REFLECT(GrdLogger) {
		GRD_MEMBER(allocator);
		GRD_MEMBER(proc);
		GRD_MEMBER(pass_level);
	}
};

GRD_DEDUP void grd_default_logger_proc(GrdLogger* logger, GrdLogInfo info, GrdUnicodeString text) {
	auto unicode_path = grd_copy_unicode_string(grd_make_string(info.loc.file));
	auto base = grd_path_basename(unicode_path);
	auto formatted = grd_sprint("%:% %()%", base, info.loc.line, GrdFmtIndent{info.indent}, text);
	grd_println(formatted);
	formatted.free();
	unicode_path.free();
}

GRD_DEDUP GrdOptional<GrdLogLevel> grd_get_cmd_line_log_level() {
	const char* log_level_val = NULL;
	for (auto i: grd_range(GRD_ARGC)) {
		if (strcmp(GRD_ARGV[i], "--grd_log_level") == 0) {
			if (i + 1 < GRD_ARGC) {
				log_level_val = GRD_ARGV[i + 1];
			}
			break;
		}
	}
	if (!log_level_val) {
		return {};
	}
	auto x = grd_reflect_type_of<GrdLogLevel>();
	if (!x) {
		return {};
	}
	auto tp = grd_type_as<GrdEnumType>(x);
	for (auto it: tp->values) {
		// compare case independently.
		auto a = it.name;
		auto b = log_level_val;
		bool match = false;
		while (*a && *b) {
			if (tolower(*a) != tolower(*b)) {
				break;
			}
			a += 1;
			b += 1;
			if (*a == '\0' && *b == '\0') {
				match = true;
			}
		}
		if (match) {
			return GrdLogLevel(it.value.u32_value);
		}
	}
	return {};
}

GRD_DEDUP GrdLogger* grd_make_default_logger() {
	auto logger = grd_make<GrdLogger>();
	logger->proc = grd_default_logger_proc;
	if (auto [level, found] = grd_get_cmd_line_log_level(); found) {
		logger->pass_level = level;
	}
	return logger;
}

GRD_DEDUP void grd_void_logger_proc(GrdLogger* logger, GrdLogInfo info, GrdUnicodeString text) {

}

GRD_DEDUP GrdLogger* grd_make_void_logger() {
	auto logger = grd_make<GrdLogger>();
	logger->proc = grd_void_logger_proc;
	return logger;
}

GRD_DEDUP GrdLogger*(*grd_make_new_logger)() = grd_make_default_logger;
GRD_DEDUP thread_local GrdLogger* __grd_thread_logger = NULL;
GRD_DEDUP thread_local GrdLogger* __grd_program_logger = NULL;
GRD_DEDUP              bool       __grd_threads_pickup_program_logger = false;

GRD_DEDUP void grd_set_thread_logger(GrdLogger* logger) {
	__grd_thread_logger = logger;
}

GRD_DEDUP void grd_set_program_logger(GrdLogger* logger) {
	__grd_program_logger = logger;
}

GRD_DEDUP GrdLogger* grd_get_program_logger() {
	if (!__grd_program_logger) {
		__grd_program_logger = grd_make_new_logger();
	}
	return __grd_program_logger;
}

GRD_DEDUP void grd_set_program_logger_to_all_threads(GrdLogger* logger) {
	grd_atomic_store(&__grd_threads_pickup_program_logger, true);
	grd_set_program_logger(logger);
}

GRD_DEDUP GrdLogger* grd_get_logger() {
	if (!__grd_thread_logger || grd_atomic_load(&__grd_threads_pickup_program_logger)) {
		__grd_thread_logger = grd_get_program_logger();
	}
	return __grd_thread_logger;
}


// Using CamelCase instead of snake_case here to avoid intersecting
//   with math log() function.

GRD_DEDUP void GrdLogAtLogger(GrdLogger* logger, GrdLogInfo info, GrdUnicodeString text) {
	if (info.level > logger->pass_level) {
		return;
	}
	logger->proc(logger, info, text);
}

GRD_DEDUP void GrdLogAtLogger(GrdLogger* logger, GrdLogInfo info, auto... args) {
	auto str = grd_sprint_unicode(logger->allocator, args...);
	GrdLogAtLogger(logger, info, (GrdUnicodeString) str);
	str.free();
}

GRD_DEDUP void GrdLogWithInfo(GrdLogInfo info, auto... args) {
	GrdLogAtLogger(grd_get_logger(), info, args...);
}

GRD_DEDUP void GrdLog(GrdCodeLoc loc, auto... args) {
	GrdLogWithInfo({ .loc = loc, }, args...);
}

GRD_DEDUP void GrdLogTrace(GrdCodeLoc loc, auto... args) {
	GrdLogWithInfo({ .loc = loc, .level = GrdLogLevel::Trace }, args...);
}

GRD_DEDUP bool grd_can_log(GrdLogger* logger, GrdLogLevel level) {
	return logger->pass_level >= level;
}

GRD_DEDUP bool grd_can_log(GrdLogLevel level) {
	return grd_can_log(grd_get_logger(), level);
}

GRD_DEDUP GrdLogInfo grd_log_info_with_loc(GrdLogInfo info, GrdCodeLoc loc) {
	info.loc = loc;
	return info;
}

// We can't use loc = grd_caller_loc() if we have parameter pack.
//   These macros help us with that situation.
#define GrdLog(...)               GrdLog(grd_current_loc(), __VA_ARGS__)
#define GrdLogTrace(...)          GrdLogTrace(grd_current_loc(), __VA_ARGS__)
#define GrdLogWithInfo(info, ...) GrdLogWithInfo(grd_log_info_with_loc(info, grd_current_loc()), __VA_ARGS__)

