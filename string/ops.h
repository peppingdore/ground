#pragma once

#include "string_def.h"
#include "coroutine.h"
#include "tuple.h"

bool is_whitespace(char32_t c) {
	// Hardcoded from Unicode's PropList.txt White_Space property list.
	if (c >= 0x0009 && c <= 0x000d) return true;
	if (c == 0x0020) return true;
	if (c == 0x0085) return true;
	if (c == 0x00A0) return true;
	if (c == 0x1680) return true;
	if (c >= 0x2000 && c <= 0x200a) return true;
	if (c == 0x2028) return true;
	if (c == 0x2029) return true;
	if (c == 0x202f) return true;
	if (c == 0x205f) return true;
	if (c == 0x3000) return true;
	return false;
}

bool is_line_break(char32_t c) {
	if (c == 0x000a) return true; // line feed
	if (c == 0x000b) return true; // vertical tab
	if (c == 0x000c) return true; // form feed
	// if (c == 0x000d) return true; // carriage return
	if (c == 0x0085) return true; // next line
	if (c == 0x2028) return true; // line separator
	if (c == 0x2029) return true; // paragraph separator
	return false;
}


auto advance(auto str, s64 shift) {
	assert(shift >= 0 && shift <= str.length);
	return decltype(str) {
		.data   = str.data   + shift,
		.length = str.length - shift
	};
}

auto slice(auto str, s64 start, s64 length) {
	assert(length <= str.length - start);
	return decltype(str) {
		.data   = str.data + start,
		.length = length
	};
}

auto slice(auto str, s64 start) {
	return slice(str, start, str.length - start);
}

bool is_blank(auto str) {
	for (auto i: str) {
		if (!is_whitespace(i)) {
			return false;
		}
	}
	return true;
}

bool starts_with(auto str, auto start) {
	if (start.length > str.length) {
		return false;
	}

	return slice(str, 0, start.length) == start;
}

bool ends_with(auto str, auto end) {
	if (end.length > str.length) {
		return false;
	}

	return slice(str, str.length - end.length) == end;
}

bool compare_ignore_case(auto a, auto b) {
	if (a.length != b.length) {
		return false;
	}

	for (auto i: range(a.length)) {
		if (u_tolower(a[i]) != u_tolower(b[i])) {
			return false;
		}
	}

	return true;
}

bool contains(auto str, auto x) {
	if (x.length > str.length) {
		return false;
	}

	for (auto i: range(str.length - x.length + 1)) {
		if (starts_with(slice(str, i), x)) {
			return true;
		}
	}

	return false;
}

bool contains(auto str, StringChar auto c) {
	for (auto it: str) {
		if (it == c) {
			return true;
		}
	}
	return false;
}

auto trim_start(auto str) {
	for (auto i: range(str.length)) {
		if (!is_whitespace(str[i])) {
			return advance(str, i);
		}
	}
	return advance(str.length);
}

auto trim_end(auto str) {
	for (auto i: reverse(range(str.length))) {
		if (!is_whitespace(str[i])) {
			return slice(0, i + 1);
		}
	}
	return slice(str, 0, 0);
}

auto trim(auto str) {
	str = trim_start(str);
	str = trim_end(str);
	return str;
}

// bool=true indicates that |should_stop| was reached.
//   If it's false, we reached end of the |str|.
auto take_until(auto str, auto should_stop) -> Tuple<decltype(str), bool> {
	for (auto i: range(str.length)) {
		if (should_stop(str[i])) {
			return { slice(str, 0, i), true };
		}
	}
	return { str, false };
}

auto split(auto str, auto predicate) -> Generator<decltype(str)> {
	s64 cursor = 0;
	for (auto i: range(str.length)) {
		if (predicate(str[i])) {
			auto x = slice(str, cursor, i - cursor);
			if (x.length > 0) {
				co_yield x;
			}
			cursor = i + 1;
		}
	}

	auto x = slice(str, cursor, str.length - cursor);
	if (x.length > 0) {
		co_yield x;
	}
}

auto split2(auto str, auto predicate) {
	for (auto i: range(str.length)) {
		if (predicate(str[i])) {
			return make_tuple(slice(str, 0, i), slice(i + 1));
		}
	}

	return make_tuple(str, decltype(str){});
}

auto split(Allocator allocator, auto str, auto predicate) {
	return to_array(allocator, split(str, predicate));
}

auto iterate_lines(auto str, bool include_line_breaks = true) -> Generator<decltype(str)> {
	s64 cursor = 0;
	for (auto i: range(str.length)) {
		if (is_line_break(str[i])) {
			co_yield slice(str, cursor, i - cursor + (include_line_breaks ? 1 : 0));
			cursor = i + 1;
		}
	}
	co_yield slice(str, cursor);
}
