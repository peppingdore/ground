#pragma once

#include "coroutine.h"
#include "string_builder.h"
#include "function.h"

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
	return last.length > 0 ? prev : last;
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

UnicodeString path_join(Allocator allocator, auto... args) {
	auto result = build_unicode_string(allocator);
	auto pieces = { make_string(args)... };
	for (auto idx: range(pieces.size())) {
		auto it = *pieces[idx];
		if (idx > 0) {
			if (it.length > 0 && is_path_sep(it[0])) {
				it = slice(it, 1);
			}
			if (it.length > 0 && is_path_sep(it[it.length - 1])) {
				it = slice(it, 0, it.length - 1);
			}
			result.append('/');
		}
		result.append(it);
	}
	return result.get_string();
}

Generator<UnicodeString> path_segments(UnicodeString path) {
	return split(path, is_path_sep);
}

UnicodeString path_parent(UnicodeString path) {
	UnicodeString last;
	for (auto it: path_segments(path)) {
		last = it;
	}
	return slice(path, 0, path.length - last.length);
}
