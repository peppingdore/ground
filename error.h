#pragma once

#include "string.h"
#include "code_location.h"
#include "string_conversion.h"
#include "code_location.h"
#include "format.h"

struct Error {
	String        text;
	Type*         type;
	CodeLocation  loc;
	Error*        prev = NULL;

	void free() {
		if (prev) {
			prev->free();
		}
		text.free();
		Free(this);
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

REFLECT(Error) {
	MEMBER(text);
	MEMBER(type);
	MEMBER(loc);
	MEMBER(prev);
}

template <typename T = Error>
T* make_error(String text, CodeLocation loc = caller_loc()) {
	auto e = make<T>();
	e->text = text;
	e->type = reflect_type_of<T>();
	e->loc  = loc;
	return e;
}

template <typename T = Error>
T* make_error(const char* str, CodeLocation loc = caller_loc()) {
	return make_error<T>(make_string(str).copy(), loc);
}

template <typename T = Error>
T* format_error(CodeLocation loc, auto... args) {
	String str = sprint<char>(c_allocator, args...);
	return make_error<T>(str, loc);
}

// We can't use loc = caller_loc() if we have parameter pack.
//   These macro help us with that situation.
#define format_error(...)      format_error(   current_loc(), __VA_ARGS__)
#define format_error_t(T, ...) format_error<T>(current_loc(), __VA_ARGS__)


#if OS_WINDOWS

struct Windows_Error: Error {
	DWORD code;
};
REFLECT(Windows_Error) {
	BASE_TYPE(Error);
	MEMBER(code);
}

Windows_Error* windows_error(CodeLocation loc = caller_loc()) {
	auto code = GetLastError();
	char buf[256];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, static_array_count(buf), NULL);
	auto e = make_error<Windows_Error>(buf, loc);
	e->code = code;
	return e;
}

#endif

#if IS_POSIX

#include <errno.h>

struct Posix_Error: Error {
	int code;
};

Posix_Error* posix_error(CodeLocation loc = caller_loc()) {
	char buf[512];
	u32  cursor = 0;
	auto num_str = to_string(errno);
	auto append = [&] (const char* str, u32 length) {
		u32 copy_length = min_u32(length, static_array_count(buf) - cursor);
		memcpy(buf + cursor, str, copy_length);
		cursor += copy_length;
	};
	auto append_c_str = [&] (const char* str) {
		append(str, strlen(str));
	};
	append_c_str("Error ");
	append(num_str.buf, num_str.length);
	append_c_str(": ");
	strerror_r(errno, buf + cursor, static_array_count(buf) - cursor);
	auto e = make_error<Posix_Error>(make_string(buf).copy(), loc);
	e->code = errno;
	return e;
}

#endif
