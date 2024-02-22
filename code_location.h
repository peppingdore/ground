#pragma once

#include "base.h"
#include <string.h>

struct Code_Location {
	s32         line;
	const char* file;

	bool operator==(Code_Location rhs) {
		return line == rhs.line && (strcmp(file, rhs.file) == 0);
	}
};

constexpr Code_Location make_code_location(s32 line, const char* file) {
	return {
		.line = line,
		.file = (char*) file,
	};
}

#if OS_WINDOWS
	#define caller_loc() make_code_location(__builtin_LINE(), __builtin_FILE())
#else
	constexpr Code_Location make_code_location(const std::source_location cpp_loc) {
		return {
			.line = (s32) cpp_loc.line(),
			.file = (char*) cpp_loc.file_name()
		};
	}
	#define caller_loc() make_code_location(std::source_location::current())
#endif

#define current_loc() make_code_location(__LINE__, __FILE__)
