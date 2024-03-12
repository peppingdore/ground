#pragma once

#include "allocator.h"
#include "string.h"
#include "range.h"
#include "array.h"

constexpr s64 STRING_BUILDER_DEFAULT_CAPACITY = 256;

template <typename T = char>
struct StringBuilder {
	Array<T> arr = { .capacity = STRING_BUILDER_DEFAULT_CAPACITY };

	T operator[](s64 i) {
		return *arr[i];
	}

	StringBuilder copy(CodeLocation loc = caller_loc()) {
		StringBuilder cp = *this;
		cp.arr = arr.copy(arr.allocator, loc);
		return cp; 
	}

	T* reserve(s64 length) {
		return arr.reserve_at_index(arr.count, length);
	}

	void append(s64 index, BaseString<T> str, CodeLocation loc = caller_loc()) {
		arr.add_at_index(index, str.data, str.length, loc);
	}

	void append(s64 index, String str, CodeLocation loc = caller_loc())
		requires std::is_same_v<T, char32_t>
	{
		T* ptr = arr.reserve_at_index(index, str.length);
		for (s64 i : range(str.length)) {
			ptr[i] = str[i];
		}
	}

	void append(BaseString<T> str, CodeLocation loc = caller_loc()) {
		append(len(arr), str, loc);
	}

	void append(s64 index, T c, CodeLocation loc = caller_loc()) {
		append(index, make_string<T>(&c, 1), loc);
	}

	void append(T c, CodeLocation loc = caller_loc()) {
		append(len(arr), c, loc);
	}

	void append(String str, CodeLocation loc = caller_loc()) requires std::is_same_v<T, char32_t> {
		append(len(arr), str, loc);
	}

	void append(const char* c_str) {
		append(make_string(c_str));
	}

	void remove(s64 index, s64 length) {
		arr.remove_at_index(index, length);
	}

	void clear() {
		arr.clear();
	}

	void free(CodeLocation loc = caller_loc()) {
		arr.free(loc);
	}

	// Does not copy the string.
	BaseString<T> get_string() {
		return make_string(arr.data, arr.count);
	}
};

template <typename T>
s64 len(StringBuilder<T> view) {
	return len(view.arr);
}

template <typename T = char>
inline StringBuilder<T> build_string(Allocator allocator = c_allocator, s64 capacity = STRING_BUILDER_DEFAULT_CAPACITY, CodeLocation loc = caller_loc()) {
	StringBuilder<T> builder;
	builder.arr.allocator = allocator;
	builder.arr.capacity  = capacity;
	return builder;
}

using UnicodeStringBuilder = StringBuilder<char32_t>;

inline UnicodeStringBuilder build_unicode_string(Allocator allocator = c_allocator, s64 capacity = STRING_BUILDER_DEFAULT_CAPACITY, CodeLocation loc = caller_loc()) {
	return build_string<char32_t>(allocator, capacity, loc);
}
