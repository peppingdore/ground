#pragma once

#include "grd_base.h"
#include "grd_hash.h"
#include <string.h>

struct GrdCodeLoc {
	s32         line = 0;
	const char* file = NULL;

	bool operator==(GrdCodeLoc rhs) {
		return line == rhs.line && (strcmp(file, rhs.file) == 0);
	}
};

constexpr GrdCodeLoc grd_make_code_loc(s32 line, const char* file) {
	return {
		.line = line,
		.file = (char*) file,
	};
}

#if GRD_OS_WINDOWS
	#define grd_caller_loc() grd_make_code_loc(__builtin_LINE(), __builtin_FILE())
#else
	#include <source_location>
	constexpr GrdCodeLoc grd_make_code_loc(const std::source_location cpp_loc) {
		return {
			.line = (s32) cpp_loc.line(),
			.file = (char*) cpp_loc.file_name()
		};
	}
	#define grd_caller_loc() grd_make_code_loc(std::source_location::current())
#endif

#define grd_current_loc() grd_make_code_loc(__LINE__, __FILE__)
