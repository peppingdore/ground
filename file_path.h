#pragma once

#include "grd_coroutine.h"
#include "function.h"
#include "string.h"

bool is_path_sep(char32_t c) {
	#if OS_WINDOWS
		return c == '\\' || c == '/';
	#else
		return c == '/';
	#endif
}

UnicodeString path_basename(UnicodeString path) {
	UnicodeString last;
	for (auto it: split(path, lambda(is_path_sep($0)))) {
		last = it;
	}
	return last;
}

UnicodeString path_stem(UnicodeString path) {
	auto base = path_basename(path);
	UnicodeString prev;
	UnicodeString last;
	for (auto it: split(base, lambda($0 == '.'))) {
		prev = last;
		last = it;
	}
	return len(last) > 0 ? prev : last;
}

UnicodeString path_ext(UnicodeString path) {
	auto base = path_basename(path);
	s64    index;
	UnicodeString last;
	for (auto it: split(base, lambda($0 == '.'))) {
		last = it;
		index += 1;
	}
	return index >= 2 ? last : UnicodeString{};
}

AllocatedUnicodeString path_join(GrdAllocator allocator, auto... args) {
	Array<char32_t> result = { .allocator = allocator };
	auto pieces = { grd_make_string(args)... };
	for (auto idx: grd_range(pieces.size())) {
		auto it = *pieces[idx];
		if (idx > 0) {
			if (it.length > 0 && is_path_sep(it[0])) {
				it = slice(it, 1);
			}
			if (it.length > 0 && is_path_sep(it[it.length - 1])) {
				it = slice(it, 0, it.length - 1);
			}
			add(&result, '/');
		}
		add(&result, it);
	}
	return result;
}

GrdGenerator<UnicodeString> path_segments(UnicodeString path) {
	return split(path, is_path_sep);
}

UnicodeString path_parent(UnicodeString path) {
	UnicodeString last;
	for (auto it: path_segments(path)) {
		last = it;
	}
	return path[0, len(path) - len(last)];
}
