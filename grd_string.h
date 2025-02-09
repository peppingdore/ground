#pragma once

#include "grd_array.h"
#include "grd_allocator.h"
#include "grd_code_location.h"
#include "grd_tuple.h"
#include "grd_byte_order.h"
#include "grd_optional.h"
#include <concepts>

template <typename T>
concept GrdStringChar = std::same_as<T, char> || std::same_as<T, char32_t>;

using GrdString = GrdSpan<char>;
using GrdUnicodeString = GrdSpan<char32_t>;
using GrdAllocatedString = GrdArray<char>;
using GrdAllocatedUnicodeString = GrdArray<char32_t>;

template <GrdStringChar T>
GRD_DEDUP constexpr GrdSpan<T> grd_make_string(const T* data, s64 length) {
	return { (T*) data, length };
}

template <GrdStringChar T>
GRD_DEDUP GrdSpan<T> grd_make_string(const T* c_str) {
	auto ptr = c_str;
	while (true) {
		if (ptr[0] == '\0') {
			break;
		}
		ptr += 1;
	}
	return { (T*) c_str, ptr - c_str };
}

GRD_DEDUP constexpr GrdString operator""_b(const char* c_str, size_t length) { 
	return { (char*) c_str, (s64) length };
}

GRD_DEDUP constexpr GrdUnicodeString operator""_b(const char32_t* c_str, size_t length) {
	return { (char32_t*) c_str, (s64) length };
}

GRD_DEDUP constexpr GrdSpan<wchar_t> operator""_b(const wchar_t* c_str, size_t length) {
	return { (wchar_t*) c_str, (s64) length };
}

GRD_DEDUP constexpr GrdSpan<char16_t> operator""_b(const char16_t* c_str, size_t length) {
	return { (char16_t*) c_str, (s64) length };
}

template <GrdStringChar T>
GRD_DEDUP void grd_append(GrdArray<T>* arr, GrdSpan<T> str, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) { 
	auto ptr = grd_reserve(arr, grd_len(str), index, loc);
	for (auto i: grd_range(grd_len(str))) {
		ptr[i] = str[i];
	}
}

GRD_DEDUP void grd_append(GrdArray<char32_t>* arr, GrdString str, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	auto ptr = grd_reserve(arr, grd_len(str), index, loc);
	for (auto i: grd_range(grd_len(str))) {
		ptr[i] = str[i];
	}
}

template <s64 N>
GRD_DEDUP void grd_append(GrdArray<char32_t>* arr, const char32_t (&str)[N], s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	grd_add(arr, GrdSpan<char32_t>{ str, N - 1 }, index, loc);
}

template <s64 N>
GRD_DEDUP void grd_append(GrdArray<char32_t>* arr, const char (&str)[N], s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	auto* ptr = grd_reserve(arr, N - 1, index, loc);
	for (auto i: grd_range(N - 1)) {
		ptr[i] = str[i];
	}
}

template <s64 N>
GRD_DEDUP void grd_append(GrdArray<char>* arr, const char (&str)[N], s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	grd_add(arr, GrdSpan<char>{ str, N - 1 }, index, loc);
}

GRD_DEDUP void grd_append(GrdArray<char32_t>* arr, const char32_t* str, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	auto s = grd_make_string(str);
	grd_add(arr, s, index, loc);
}

GRD_DEDUP void grd_append(GrdArray<char32_t>* arr, const char* str, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	auto s = grd_make_string(str);
	grd_append(arr, s, index, loc);
}

GRD_DEDUP void grd_append(GrdArray<char>* arr, const char* str, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	auto s = grd_make_string(str);
	grd_add(arr, s, index, loc);
}

template <GrdStringChar T>
GRD_DEDUP T* grd_copy_c_str(GrdSpan<T> str, GrdAllocator allocator = c_allocator) {
	auto cp = GrdAlloc<T>(allocator, grd_len(str) + 1);
	memcpy(cp, str.data, grd_len(str) * sizeof(T));
	cp[grd_len(str)] = '\0';
	return cp;
}

template <GrdStringChar T>
GRD_DEDUP GrdArray<T> grd_copy_string(GrdAllocator allocator, GrdSpan<T> str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_copy_array(allocator, str, loc);
}

template <GrdStringChar T>
GRD_DEDUP GrdArray<T> grd_copy_string(GrdSpan<T> str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_copy_string(c_allocator, str, loc);
}

GRD_DEDUP GrdAllocatedUnicodeString grd_copy_unicode_string(GrdAllocator allocator, GrdString str) {
	GrdArray<char32_t> result = { .allocator = allocator };
	grd_append(&result, str);
	return result;
}

GRD_DEDUP GrdAllocatedUnicodeString grd_copy_unicode_string(GrdString str) {
	return grd_copy_unicode_string(c_allocator, str);
}

GRD_DEDUP bool grd_is_whitespace(char32_t c) {
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

GRD_DEDUP bool grd_is_line_break(char32_t c) {
	if (c == 0x000a) return true; // line feed
	if (c == 0x000b) return true; // vertical tab
	if (c == 0x000c) return true; // form feed
	if (c == 0x000d) return true; // carriage return
	if (c == 0x0085) return true; // next line
	if (c == 0x2028) return true; // line separator
	if (c == 0x2029) return true; // paragraph separator
	return false;
}

template <GrdStringChar T>
GRD_DEDUP bool grd_is_blank(GrdSpan<T> str) {
	for (T c: str) {
		if (!grd_is_whitespace(c)) {
			return false;
		}
	}
	return true;
}

template <GrdStringChar T, GrdStringChar U>
GRD_DEDUP bool grd_starts_with(GrdSpan<T> str, GrdSpan<U> start) {
	if (grd_len(start) > grd_len(str)) {
		return false;
	}
	return str[0, grd_len(start)] == start;
}

template <GrdStringChar T, GrdStringChar U, s64 N>
GRD_DEDUP bool grd_starts_with(GrdSpan<T> str, const U (&start)[N]) {
	return grd_starts_with(str, GrdSpan<U>{ (U*) start, N - 1 });
}

template <GrdStringChar T, GrdStringChar U>
GRD_DEDUP bool grd_ends_with(GrdSpan<T> str, GrdSpan<U> end) {
	if (grd_len(end) > grd_len(str)) {
		return false;
	}
	return str[-grd_len(end), {}] == end;
}

GRD_DEDUP char32_t grd_ascii_to_lower(char32_t c) {
	return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

template <GrdStringChar T, GrdStringChar U>
GRD_DEDUP bool grd_compare_ignore_case_ascii(GrdSpan<T> a, GrdSpan<U> b) {
	if (grd_len(a) != grd_len(b)) {
		return false;
	}
	for (auto i: grd_range(grd_len(a))) {
		if (grd_ascii_to_lower(a[i]) != grd_ascii_to_lower(b[i])) {
			return false;
		}
	}
	return true;
}

template <GrdStringChar T>
GRD_DEDUP GrdSpan<T> grd_trim_start(GrdSpan<T> str) {
	for (auto i: grd_range(grd_len(str))) {
		if (!grd_is_whitespace(str[i])) {
			return str[i, grd_len(str)];
		}
	}
	return str[grd_len(str), grd_len(str)];
}

template <GrdStringChar T>
GRD_DEDUP GrdSpan<T> grd_trim_end(GrdSpan<T> str) {
	for (auto i: grd_reverse(grd_range(grd_len(str)))) {
		if (!grd_is_whitespace(str[i])) {
			return str[0, i + 1];
		}
	}
	return str[0, 0];
}

template <GrdStringChar T>
GRD_DEDUP GrdSpan<T> grd_trim(GrdSpan<T> str) {
	str = grd_trim_start(str);
	str = grd_trim_end(str);
	return str;
}

// bool=true indicates that |should_stop| was reached.
//   If it's false, we reached end of the |str|.

template <GrdStringChar T>
GRD_DEDUP GrdTuple<GrdSpan<T>, bool> grd_take_until(GrdSpan<T> str, auto should_stop) {
	for (auto i: grd_range(grd_len(str))) {
		if (should_stop(str[i])) {
			return { str[0, i], true };	
		}
	}
	return { str, false };
}

template <GrdStringChar T>
GRD_DEDUP GrdGenerator<GrdSpan<T>> grd_split(GrdSpan<T> str, auto predicate) {
	s64 cursor = 0;
	for (auto i: grd_range(grd_len(str))) {
		if (predicate(str[i])) {
			auto x = str[cursor, i];
			co_yield x;
			cursor = i + 1;
		}
	}

	auto x = str[cursor, grd_len(str)];
	co_yield x;
}

template <GrdStringChar T>
GRD_DEDUP GrdTuple<GrdSpan<T>, GrdSpan<T>> grd_split2(GrdSpan<T> str, auto predicate) {
	for (auto i: grd_range(grd_len(str))) {
		if (predicate(str[i])) {
			return { str[0, i], str[i + 1, grd_len(str)] };
		}
	}
	return { str, GrdSpan<T>{} }; 
}

// Returns the length of the line break at |idx|.
// 0 if no line break, 1 if regular line break, 2 if \r\n.
template <GrdStringChar T>
GRD_DEDUP s64 grd_get_line_break_len(GrdSpan<T> str, s64 idx) {
	if (idx < grd_len(str)) {
		if (str[idx] == '\r' && (idx + 1 < grd_len(str) && str[idx + 1] == '\n')) {
			return 2;
		}
		if (grd_is_line_break(str[idx])) {
			return 1;
		}
	}
	return 0;
}

template <GrdStringChar T>
GRD_DEDUP GrdGenerator<GrdSpan<T>> grd_iterate_lines(GrdSpan<T> str, bool include_line_breaks = true) {
	s64 cursor = 0;
	s64 i = 0;
	while (i < grd_len(str)) {
		auto line_break_len = grd_get_line_break_len(str, i);
		if (line_break_len != 0) {
			co_yield str[cursor, i + (include_line_breaks ? line_break_len : 0)];
			cursor = i + line_break_len;
			i += line_break_len - 1;
		}
		i += 1;
	}
	co_yield str[cursor, grd_len(str)];
}

template <GrdStringChar T, GrdStringChar U>
GRD_DEDUP GrdSpan<T> grd_remove_prefix(GrdSpan<T> str, GrdSpan<U> prefix) {
	if (grd_starts_with(str, prefix)) {
		return str[grd_len(prefix), grd_len(str)];
	}
	return str;
}

template <GrdStringChar T, GrdStringChar U>
GRD_DEDUP GrdSpan<T> grd_remove_suffix(GrdSpan<T> str, GrdSpan<U> suffix) {
	if (grd_ends_with(str, suffix)) {
		return str[0, grd_len(str) - grd_len(suffix)];
	}
	return str;
}

template <GrdStringChar T>
GRD_DEDUP bool grd_contains(GrdSpan<T> str, const T* substr) {
	auto rhs = grd_make_string(substr);
	return grd_contains(str, rhs);
}


GRD_DEDUP s32 grd_utf8_char_size(char32_t c) {
	if (c <= 0x007F) return 1;
	if (c <= 0x07FF) return 2;
	if (c <= 0xFFFF) return 3; 
	                 return 4;
}

GRD_DEDUP s64 grd_utf8_length(GrdUnicodeString str) {
	s64 length = 0;
	for (auto i: grd_range(grd_len(str))) {
		length += grd_utf8_char_size(str[i]);
	}
	return length;
}

GRD_DEDUP void grd_encode_utf8(GrdUnicodeString str, char* buf) {
	char* ptr = buf;
	for (auto i: grd_range(grd_len(str))) {
		auto c = str[i];
		auto char_size = grd_utf8_char_size(c);

		switch (char_size) {
			case 1:
				ptr[0] = c;
				break;
			case 2:
				ptr[0] = (((c >> 6)  & 0b0001'1111) | 0b1100'0000);
				ptr[1] = (( c        & 0b0011'1111) | 0b1000'0000);
				break;
			case 3:
				ptr[0] = (((c >> 12) & 0b0000'1111) | 0b1110'0000);
				ptr[1] = (((c >> 6)  & 0b0011'1111) | 0b1000'0000);
				ptr[2] = (( c        & 0b0011'1111) | 0b1000'0000);
				break;
			case 4:
				ptr[0] = (((c >> 24) & 0b0000'0111) | 0b1111'0000);
				ptr[1] = (((c >> 12) & 0b0011'1111) | 0b1000'0000);
				ptr[2] = (((c >> 6)  & 0b0011'1111) | 0b1000'0000);
				ptr[3] = (( c        & 0b0011'1111) | 0b1000'0000);
				break;
		}
		ptr += char_size;
	}
}

// Zero terminated, terminator is at [length], not [length - 1].
GRD_DEDUP GrdAllocatedString grd_encode_utf8(GrdAllocator allocator, GrdUnicodeString str, GrdCodeLoc loc = grd_caller_loc()) {
	s64 length = grd_utf8_length(str);
	GrdAllocatedString result = {
		.allocator = allocator,
		.loc = loc,
	};
	auto data = grd_reserve(&result, length + 1);
	grd_encode_utf8(str, data);
	data[length] = '\0';
	result.count = length;
	return result;
}

GRD_DEDUP GrdAllocatedString grd_encode_utf8(GrdUnicodeString str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_encode_utf8(c_allocator, str, loc);
}

GRD_DEDUP s32 grd_utf16_char_size(char32_t c) {
	return c >= 0x10000 ? 2 : 1;
}

GRD_DEDUP GrdArray<char16_t> grd_encode_utf16(GrdAllocator allocator, GrdUnicodeString str, GrdCodeLoc loc = grd_caller_loc()) {
	
	s64 length = 0;
	for (auto i: grd_range(grd_len(str))) {
		length += grd_utf16_char_size(str[i]);
	}

	GrdArray<char16_t> result = { .allocator = allocator };
	char16_t* data = grd_reserve(&result, length + 1);
	data[length] = '\0';

	auto ptr = data;
	for (auto i: grd_range(grd_len(str))) {
		auto c = str[i];
		auto char_size = grd_utf16_char_size(c);
		if (char_size > 1) {
			ptr[0] = ((c >> 10)   + 0xD800);
			ptr[1] = ((c & 0x3FF) + 0xDC00);
		} else {
			ptr[0] = c;
		}
		ptr += char_size;
	}

	result.count = length;
	return result;
}

GRD_DEDUP GrdArray<char16_t> grd_encode_utf16(GrdUnicodeString str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_encode_utf16(c_allocator, str, loc);
}

GRD_DEDUP GrdAllocatedUnicodeString grd_decode_utf8(GrdAllocator allocator, GrdString utf8, GrdCodeLoc loc = grd_caller_loc()) {

	s64 length = 0;
	for (auto c: utf8) {
		if ((c & 0xC0) == 0x80) {
			continue;
		}
		length += 1;
	}

	GrdArray<char32_t> result = { .allocator = allocator };
	char32_t* data = grd_reserve(&result, length + 1);
	data[length] = '\0';

	auto dst = data;
	auto src = utf8.data;
	while (dst < data + length && src < utf8.data + grd_len(utf8)) {  
		s32 size = 1;
		if        ((src[0] & 0xE0) == 0xC0) {
			size = 2;
		} else if ((src[0] & 0xF0) == 0xE0) {
			size = 3;
		} else if ((src[0] & 0xF8) == 0xF0) {
			size = 4;
		}

		if (src + size > utf8.data + grd_len(utf8)) {
			dst[0] = '?';
			break;
		}

		switch (size) {
			case 1:
				dst[0] = src[0] & 0x7F;
				break;
			case 2:
				dst[0] =
					char32_t(src[0] & 0x0F) << 12 |
					char32_t(src[1] & 0x3F) << 6;
				break;
			case 3:
				dst[0] =
					char32_t(src[0] & 0x0F) << 12 |
					char32_t(src[1] & 0x3F) << 6 |
					char32_t(src[3] & 0x3F);
				break;
			case 4:
				dst[0] =
					char32_t(src[0] & 0x08) << 18 |
					char32_t(src[1] & 0x3F) << 12 |
					char32_t(src[2] & 0x3F) << 6  |
					char32_t(src[3] & 0x3F);
				break;
		}
		dst += 1;
		src += size;
	}
	result.count = length;
	return result;
}

GRD_DEDUP GrdAllocatedUnicodeString grd_decode_utf8(GrdAllocator allocator, const char* c_str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_decode_utf8(allocator, grd_make_string(c_str), loc);
}

GRD_DEDUP GrdAllocatedUnicodeString grd_decode_utf8(const char* c_str, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_decode_utf8(c_allocator, c_str, loc);
}

GRD_DEDUP GrdAllocatedUnicodeString grd_decode_utf16(GrdAllocator allocator, const char16_t* str, s64 length = -1, GrdOptional<GrdByteOrder> bo = {}, GrdCodeLoc loc = grd_caller_loc()) {

	if (length < 0) {
		auto ptr = str;
		while (*ptr != '\0') {
			ptr += 1;
		}
		length = ptr - str;
	}

	if (!bo.has_value) {
		if (length > 0 && str[0] == 0xFEFF) {
			bo = GRD_BYTE_ORDER_BIG_ENDIAN;
		} else {
			bo = GRD_BYTE_ORDER_LITTLE_ENDIAN;
		}
	}
	assert(bo.has_value);

	s64 result_length = 0;
	for (auto i: grd_range(length)) {
		auto c = str[i];
		if (bo != GRD_BYTE_ORDER_LITTLE_ENDIAN) {
			grd_swap_endianness(&c);
		}
		if ((c & 0xFC) == 0xD8) {
			continue;
		}
		result_length += 1;
	}

	GrdArray<char32_t> result = { .allocator = allocator };
	char32_t* data = grd_reserve(&result, result_length + 1);
	data[result_length] = '\0';
	result.count = result_length;

	auto dst = data;
	auto src = str;
	while (src < str + length && dst < data + result_length) { 
		auto c = src[0];
		if (bo != GRD_BYTE_ORDER_LITTLE_ENDIAN) {
			grd_swap_endianness(&c);
		}

		if ((c & 0xFC) == 0xD8) {
			if (src + 2 > str + length) {
				dst[0] = '?';
				break;
			}
			dst[0] =
				char32_t(src[0] - 0xD800) << 10 |
				char32_t(src[1] - 0xDC00) + 0x10000;
			src += 2;
		} else {
			dst[0] = src[0];
			src += 1;
		}
		dst += 1;
	}

	return result;
}

GRD_DEDUP GrdAllocatedUnicodeString grd_decode_utf16(const char16_t* str, s64 length = -1, GrdOptional<GrdByteOrder> bo = {}, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_decode_utf16(c_allocator, str, length, bo, loc);
}
