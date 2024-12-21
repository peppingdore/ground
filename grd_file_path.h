#pragma once

#include "grd_coroutine.h"
#include "grd_function.h"
#include "grd_string.h"

GRD_DEDUP bool grd_is_path_sep(char32_t c) {
	#if GRD_OS_WINDOWS
		return c == '\\' || c == '/';
	#else
		return c == '/';
	#endif
}

GRD_DEDUP GrdUnicodeString grd_path_basename(GrdUnicodeString path) {
	GrdUnicodeString last;
	for (auto it: grd_split(path, grd_lambda(x, grd_is_path_sep(x)))) {
		last = it;
	}
	return last;
}

GRD_DEDUP GrdUnicodeString grd_path_stem(GrdUnicodeString path) {
	auto base = grd_path_basename(path);
	GrdUnicodeString prev;
	GrdUnicodeString last;
	for (auto it: grd_split(base, grd_lambda(x, x == '.'))) {
		prev = last;
		last = it;
	}
	return grd_len(last) > 0 ? prev : last;
}

GRD_DEDUP GrdUnicodeString grd_path_ext(GrdUnicodeString path) {
	auto base = grd_path_basename(path);
	s64    index;
	GrdUnicodeString last;
	for (auto it: grd_split(base, grd_lambda(x, x == '.'))) {
		last = it;
		index += 1;
	}
	return index >= 2 ? last : GrdUnicodeString{};
}

GRD_DEDUP GrdAllocatedUnicodeString grd_path_join(GrdAllocator allocator, GrdSpan<GrdUnicodeString> args) {
	GrdArray<char32_t> result = { .allocator = allocator };
	for (auto idx: grd_range(grd_len(args))) {
		auto it = args[idx];
		if (idx > 0) {
			if (grd_len(it) > 0 && grd_is_path_sep(it[0])) {
				it = it[{1, {}}];
			}
			if (grd_len(it) > 0 && grd_is_path_sep(it[-1])) {
				it = it[{0, -1}];
			}
			grd_add(&result, '/');
		}
		grd_add(&result, it);
	}
	return result;
}

GRD_DEDUP GrdAllocatedUnicodeString grd_path_join(GrdAllocator allocator, std::initializer_list<GrdUnicodeString> args) {
	return grd_path_join(allocator, grd_make_span(args));
}

GRD_DEDUP GrdAllocatedUnicodeString grd_path_join(std::initializer_list<GrdUnicodeString> args) {
	return grd_path_join(c_allocator, args);
}

GRD_DEDUP GrdGenerator<GrdUnicodeString> grd_path_segments(GrdUnicodeString path) {
	return grd_split(path, grd_is_path_sep);
}

GRD_DEDUP GrdUnicodeString grd_path_parent(GrdUnicodeString path) {
	GrdUnicodeString last;
	for (auto it: grd_path_segments(path)) {
		last = it;
	}
	return path[{0, grd_len(path) - grd_len(last)}];
}
