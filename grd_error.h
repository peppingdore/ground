#pragma once

#include "grd_string.h"
#include "grd_code_location.h"
#include "grd_number_string_conversion.h"
#include "grd_code_location.h"
#include "grd_format.h"

struct GrdError {
	GrdAllocatedString text;
	GrdType*           type;
	GrdCodeLoc         loc;
	GrdFreeList*       free_list = NULL;

	void free() {
		grd_free_list_free(free_list);
		text.free();
		GrdFree(this);
	}
};

GRD_DEDUP void grd_type_format(GrdFormatter* formatter, GrdError* e, GrdString spec) {
	grd_format(formatter, "% at %", e->text, e->loc);
}

GRD_REFLECT(GrdError) {
	GRD_MEMBER(text);
	GRD_MEMBER(type);
		GRD_TAG(GrdRealTypeMember{});
	GRD_MEMBER(loc);
	GRD_MEMBER(free_list);
}

template <typename T = GrdError>
GRD_DEDUP T* grd_make_error(GrdAllocatedString text, GrdCodeLoc loc = grd_caller_loc()) {
	auto e = grd_make<T>();
	e->text = text;
	e->type = grd_reflect_type_of<T>();
	e->loc  = loc;
	return e;
}

template <typename T = GrdError>
GRD_DEDUP T* grd_make_error(const char* str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_make_error<T>(grd_copy_string(grd_make_string(str)), loc); 
}

template <typename T = GrdError>
GRD_DEDUP T* grd_format_error(GrdCodeLoc loc, auto... args) {
	GrdAllocatedString str = sprint<char>(c_allocator, args...);
	return grd_make_error<T>(str, loc);
}

// We can't use loc = grd_caller_loc() if we have parameter pack.
//   These macro help us with that situation.
#define grd_format_error(...)      grd_format_error(   grd_current_loc(), __VA_ARGS__)
#define grd_format_error_t(T, ...) grd_format_error<T>(grd_current_loc(), __VA_ARGS__)


#if GRD_OS_WINDOWS

struct GrdWindowsError: GrdError {
	GRD_WIN_DWORD code;
};
GRD_REFLECT(GrdWindowsError) {
	GRD_BASE_TYPE(GrdError);
	GRD_MEMBER(code);
}

GRD_DEDUP GrdWindowsError* grd_windows_error(GrdCodeLoc loc = grd_caller_loc()) {
	auto code = GetLastError();
	char buf[256];
	FormatMessageA(GRD_WIN_FORMAT_MESSAGE_FROM_SYSTEM | GRD_WIN_FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, code, GRD_WIN_MAKELANGID(GRD_WIN_LANG_NEUTRAL, GRD_WIN_SUBLANG_DEFAULT),
		buf, grd_static_array_count(buf), NULL);
	auto e = grd_make_error<GrdWindowsError>(buf, loc);
	e->code = code;
	return e;
}

#endif

#if GRD_IS_POSIX

#include <errno.h>

struct GrdPosixError: GrdError {
	int code;
};

GRD_DEDUP GrdPosixError* grd_posix_error(GrdCodeLoc loc = grd_caller_loc()) {
	char buf[512];
	u32  cursor = 0;
	auto num_str = grd_to_string(errno);
	auto append = [&] (const char* str, u32 length) {
		u32 copy_length = grd_min_u32(length, grd_static_array_count(buf) - cursor);
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
	auto e = grd_make_error<GrdPosixError>(grd_copy_string(grd_make_string(buf)), loc);
	e->code = errno;
	return e;
}

#endif
