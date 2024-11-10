#pragma once

#include "grd_coroutine.h"
#include "grd_function.h"
#include "grd_string.h"

bool grd_is_path_sep(char32_t c) {
	#if GRD_OS_WINDOWS
		return c == '\\' || c == '/';
	#else
		return c == '/';
	#endif
}

GrdUnicodeString grd_path_basename(GrdUnicodeString path) {
	GrdUnicodeString last;
	for (auto it: grd_split(path, grd_lambda(x, grd_is_path_sep(x)))) {
		last = it;
	}
	return last;
}

GrdUnicodeString grd_path_stem(GrdUnicodeString path) {
	auto base = grd_path_basename(path);
	GrdUnicodeString prev;
	GrdUnicodeString last;
	for (auto it: grd_split(base, grd_lambda(x, x == '.'))) {
		prev = last;
		last = it;
	}
	return grd_len(last) > 0 ? prev : last;
}

GrdUnicodeString path_ext(GrdUnicodeString path) {
	auto base = grd_path_basename(path);
	s64    index;
	GrdUnicodeString last;
	for (auto it: grd_split(base, grd_lambda(x, x == '.'))) {
		last = it;
		index += 1;
	}
	return index >= 2 ? last : GrdUnicodeString{};
}

GrdAllocatedUnicodeString grd_path_join(GrdAllocator allocator, auto... args) {
	GrdArray<char32_t> result = { .allocator = allocator };
	auto pieces = { grd_make_string(args)... };
	for (auto idx: grd_range(pieces.size())) {
		auto it = *pieces[idx];
		if (idx > 0) {
			if (it.length > 0 && grd_is_path_sep(it[0])) {
				it = slice(it, 1);
			}
			if (it.length > 0 && grd_is_path_sep(it[it.length - 1])) {
				it = slice(it, 0, it.length - 1);
			}
			grd_add(&result, '/');
		}
		grd_add(&result, it);
	}
	return result;
}

GrdGenerator<GrdUnicodeString> grd_path_segments(GrdUnicodeString path) {
	return grd_split(path, grd_is_path_sep);
}

GrdUnicodeString grd_path_parent(GrdUnicodeString path) {
	GrdUnicodeString last;
	for (auto it: grd_path_segments(path)) {
		last = it;
	}
	return path[0, grd_len(path) - grd_len(last)];
}
