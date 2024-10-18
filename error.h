#pragma once

#include "string.h"
#include "grd_code_location.h"
#include "number_string_conversion.h"
#include "grd_code_location.h"
#include "format.h"

struct Error {
	AllocatedString text;
	Type*           type;
	GrdCodeLoc    loc;
	void          (*on_free)(Error*) = NULL;
	Error*          prev = NULL;

	void free() {
		if (prev) {
			prev->free();
		}
		if (on_free) {
			on_free(this);
		}
		text.free();
		GrdFree(this);
	}

	Error* because(Error* error) {
		prev = error;
		return this;
	}
};

inline void type_format(Formatter* formatter, Error* e, String spec) {
	bool is_nested = contains(spec, "nested_error"_b);

	String prefix = ""_b;
	if (is_nested) {
		prefix = "because "_b;
	}
	format(formatter, "%% at %", prefix, e->text, e->loc);

	if (e->prev) {
		if (!is_nested) {
			formatter->indentation += 1;
		}
		format(formatter, "\n");
		format(formatter, "%(nested_error)", *e->prev);
		if (!is_nested) {
			formatter->indentation -= 1;
		}
	}
}

GRD_REFLECT(Error) {
	GRD_MEMBER(text);
	GRD_MEMBER(type);
	GRD_MEMBER(loc);
	GRD_MEMBER(prev);
}

template <typename T = Error>
T* grd_make_error(AllocatedString text, GrdCodeLoc loc = grd_caller_loc()) {
	auto e = grd_make<T>();
	e->text = text;
	e->type = grd_reflect_type_of<T>();
	e->loc  = loc;
	return e;
}

template <typename T = Error>
T* grd_make_error(const char* str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_make_error<T>(copy_string(grd_make_string(str)), loc); 
}

template <typename T = Error>
T* format_error(GrdCodeLoc loc, auto... args) {
	AllocatedString str = sprint<char>(c_allocator, args...);
	return grd_make_error<T>(str, loc);
}

// We can't use loc = grd_caller_loc() if we have parameter pack.
//   These macro help us with that situation.
#define format_error(...)      format_error(   grd_current_loc(), __VA_ARGS__)
#define format_error_t(T, ...) format_error<T>(grd_current_loc(), __VA_ARGS__)


#if GRD_OS_WINDOWS

struct Windows_Error: Error {
	DWORD code;
};
GRD_REFLECT(Windows_Error) {
	GRD_BASE_TYPE(Error);
	GRD_MEMBER(code);
}

Windows_Error* windows_error(GrdCodeLoc loc = grd_caller_loc()) {
	auto code = GetLastError();
	char buf[256];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, grd_static_array_count(buf), NULL);
	auto e = grd_make_error<Windows_Error>(buf, loc);
	e->code = code;
	return e;
}

#endif

#if GRD_IS_POSIX

#include <errno.h>

struct Posix_Error: Error {
	int code;
};

Posix_Error* posix_error(GrdCodeLoc loc = grd_caller_loc()) {
	char buf[512];
	u32  cursor = 0;
	auto num_str = to_string(errno);
	auto append = [&] (const char* str, u32 length) {
		u32 copy_length = min_u32(length, grd_static_array_count(buf) - cursor);
		memcpy(buf + cursor, str, copy_length);
		cursor += copy_length;
	};
	auto append_c_str = [&] (const char* str) {
		append(str, strlen(str));
	};
	append_c_str("Error ");
	append(num_str.buf, num_str.length);
	append_c_str(": ");
	strerror_r(errno, buf + cursor, grd_static_array_count(buf) - cursor);
	auto e = grd_make_error<Posix_Error>(copy_string(grd_make_string(buf)), loc);
	e->code = errno;
	return e;
}

#endif
