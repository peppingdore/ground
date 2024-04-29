#pragma once

#include "array.h"
#include "allocator.h"
#include "code_location.h"
#include "tuple.h"
#include "byte_order.h"
#include "optional.h"
#include <concepts>

template <typename T>
concept StringChar = std::same_as<T, char> || std::same_as<T, char32_t>;

using String = Span<char>;
using UnicodeString = Span<char32_t>;
using AllocatedString = Array<char>;
using AllocatedUnicodeString = Array<char32_t>;

template <StringChar T>
constexpr Span<T> make_string(const T* data, s64 length) {
	return { (T*) data, length };
}

template <StringChar T>
Span<T> make_string(const T* c_str) {
	auto ptr = c_str;
	while (true) {
		if (ptr[0] == '\0') {
			break;
		}
		ptr += 1;
	}
	return { (T*) c_str, ptr - c_str };
}

constexpr String operator""_b(const char* c_str, size_t length) { 
	return { (char*) c_str, (s64) length };
}

constexpr UnicodeString operator""_b(const char32_t* c_str, size_t length) {
	return { (char32_t*) c_str, (s64) length };
}

template <StringChar T>
void append(Array<T>* arr, Span<T> str, s64 index = -1, CodeLocation loc = caller_loc()) { 
	auto ptr = reserve(arr, len(str), index, loc);
	for (auto i: range(len(str))) {
		ptr[i] = str[i];
	}
}

void append(Array<char32_t>* arr, String str, s64 index = -1, CodeLocation loc = caller_loc()) {
	auto ptr = reserve(arr, len(str), index, loc);
	for (auto i: range(len(str))) {
		ptr[i] = str[i];
	}
}

template <s64 N>
void append(Array<char32_t>* arr, const char32_t (&str)[N], s64 index = -1, CodeLocation loc = caller_loc()) {
	add(arr, Span<char32_t>{ str, N - 1 }, index, loc);
}

template <s64 N>
void append(Array<char32_t>* arr, const char (&str)[N], s64 index = -1, CodeLocation loc = caller_loc()) {
	auto* ptr = reserve(arr, N - 1, index, loc);
	for (auto i: range(N - 1)) {
		ptr[i] = str[i];
	}
}

template <s64 N>
void append(Array<char>* arr, const char (&str)[N], s64 index = -1, CodeLocation loc = caller_loc()) {
	add(arr, Span<char>{ str, N - 1 }, index, loc);
}

void append(Array<char32_t>* arr, const char32_t* str, s64 index = -1, CodeLocation loc = caller_loc()) {
	auto s = make_string(str);
	add(arr, s, index, loc);
}

void append(Array<char32_t>* arr, const char* str, s64 index = -1, CodeLocation loc = caller_loc()) {
	auto s = make_string(str);
	append(arr, s, index, loc);
}

void append(Array<char>* arr, const char* str, s64 index = -1, CodeLocation loc = caller_loc()) {
	auto s = make_string(str);
	add(arr, s, index, loc);
}

template <StringChar T>
T* copy_c_str(Span<T> str, Allocator allocator = c_allocator) {
	auto cp = Alloc<T>(allocator, len(str) + 1);
	memcpy(cp, str.data, len(str) * sizeof(T));
	cp[len(str)] = '\0';
	return cp;
}

template <StringChar T>
Array<T> copy_string(Span<T> str, Allocator allocator = c_allocator, CodeLocation loc = caller_loc()) {
	Array<T> result = { .allocator = allocator };
	add(&result, str);
	return result;
}

AllocatedUnicodeString copy_unicode_string(Allocator allocator, String str) {
	Array<char32_t> result = { .allocator = allocator };
	append(&result, str);
	return result;
}

AllocatedUnicodeString copy_unicode_string(String str) {
	return copy_unicode_string(c_allocator, str);
}

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
	if (c == 0x000d) return true; // carriage return
	if (c == 0x0085) return true; // next line
	if (c == 0x2028) return true; // line separator
	if (c == 0x2029) return true; // paragraph separator
	return false;
}

template <StringChar T>
bool is_blank(Span<T> str) {
	for (T c: str) {
		if (!is_whitespace(c)) {
			return false;
		}
	}
	return true;
}

template <StringChar T, StringChar U>
bool starts_with(Span<T> str, Span<U> start) {
	if (len(start) > len(str)) {
		return false;
	}
	return str[0, len(start)] == start;
}

template <StringChar T, StringChar U>
bool ends_with(Span<T> str, Span<U> end) {
	if (len(end) > len(str)) {
		return false;
	}
	return str[-len(end), len(end)] == end;
}

char32_t ascii_to_lower(char32_t c) {
	return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

template <StringChar T, StringChar U>
bool compare_ignore_case_ascii(Span<T> a, Span<U> b) {
	if (len(a) != len(b)) {
		return false;
	}
	for (auto i: range(len(a))) {
		if (ascii_to_lower(a[i]) != ascii_to_lower(b[i])) {
			return false;
		}
	}
	return true;
}

template <StringChar T>
Span<T> trim_start(Span<T> str) {
	for (auto i: range(len(str))) {
		if (!is_whitespace(str[i])) {
			return str[i, len(str)];
		}
	}
	return str[len(str), len(str)];
}

template <StringChar T>
Span<T> trim_end(Span<T> str) {
	for (auto i: reverse(range(len(str)))) {
		if (!is_whitespace(str[i])) {
			return str[0, i + 1];
		}
	}
	return str[0, 0];
}

template <StringChar T>
Span<T> trim(Span<T> str) {
	str = trim_start(str);
	str = trim_end(str);
	return str;
}

// bool=true indicates that |should_stop| was reached.
//   If it's false, we reached end of the |str|.

template <StringChar T>
Tuple<Span<T>, bool> take_until(Span<T> str, auto should_stop) {
	for (auto i: range(len(str))) {
		if (should_stop(str[i])) {
			return { str[0, i], true };	
		}
	}
	return { str, false };
}

template <StringChar T>
Generator<Span<T>> split(Span<T> str, auto predicate) {
	s64 cursor = 0;
	for (auto i: range(len(str))) {
		if (predicate(str[i])) {
			auto x = str[cursor, i];
			co_yield x;
			cursor = i + 1;
		}
	}

	auto x = str[cursor, len(str)];
	co_yield x;
}

template <StringChar T>
Tuple<Span<T>, Span<T>> split2(Span<T> str, auto predicate) {
	for (auto i: range(len(str))) {
		if (predicate(str[i])) {
			return { str[0, i], str[i + 1, len(str)] };
		}
	}
	return { str, Span<T>{} }; 
}

template <StringChar T>
Generator<Span<T>> iterate_lines(Span<T> str, bool include_line_breaks = true) {
	s64 cursor = 0;
	s64 i = 0;
	while (i < len(str)) {
		if (is_line_break(str[i])) {
			if (str[i] == '\r' && (i + 1 < len(str) && str[i+1] == '\n')) {
				co_yield str[cursor, i + (include_line_breaks ? 2 : 0)];
				cursor = i + 2;
				i += 1;
			} else {
				co_yield str[cursor, i + (include_line_breaks ? 1 : 0)];
				cursor = i + 1;
			}
		}
		i += 1;
	}
	co_yield str[cursor, len(str)];
}

template <StringChar T, StringChar U>
Span<T> remove_prefix(Span<T> str, Span<U> prefix) {
	if (starts_with(str, prefix)) {
		return str[len(prefix), len(str)];
	}
	return str;
}

template <StringChar T, StringChar U>
Span<T> remove_suffix(Span<T> str, Span<U> suffix) {
	if (ends_with(str, suffix)) {
		return str[0, len(str) - len(suffix)];
	}
	return str;
}


s32 utf8_char_size(char32_t c) {
	if (c <= 0x007F) return 1;
	if (c <= 0x07FF) return 2;
	if (c <= 0xFFFF) return 3; 
	                 return 4;
}

s64 utf8_length(UnicodeString str) {
	s64 length = 0;
	for (auto i: range(len(str))) {
		length += utf8_char_size(str[i]);
	}
	return length;
}

void encode_utf8(UnicodeString str, char* buf) {
	char* ptr = buf;
	for (auto i: range(len(str))) {
		auto c = str[i];
		auto char_size = utf8_char_size(c);

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

// Zero terminated, terminator is at index length, not length - 1.
AllocatedString encode_utf8(Allocator allocator, UnicodeString str, CodeLocation loc = caller_loc()) {
	s64 length = utf8_length(str);
	AllocatedString result = {
		.allocator = allocator,
		.loc = loc,
	};
	auto data = reserve(&result, length + 1);
	encode_utf8(str, data);
	data[length] = '\0';
	result.count = length;
	return result;
}

AllocatedString encode_utf8(UnicodeString str, CodeLocation loc = caller_loc()) {
	return encode_utf8(c_allocator, str, loc);
}

s32 utf16_char_size(char32_t c) {
	return c >= 0x10000 ? 2 : 1;
}

Array<char16_t> encode_utf16(Allocator allocator, UnicodeString str, CodeLocation loc = caller_loc()) {
	
	s64 length = 0;
	for (auto i: range(len(str))) {
		length += utf16_char_size(str[i]);
	}

	Array<char16_t> result = { .allocator = allocator };
	char16_t* data = reserve(&result, length + 1);
	data[length] = '\0';

	auto ptr = data;
	for (auto i: range(len(str))) {
		auto c = str[i];
		auto char_size = utf16_char_size(c);
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

Array<char16_t> encode_utf16(UnicodeString str, CodeLocation loc = caller_loc()) {
	return encode_utf16(c_allocator, str, loc);
}

AllocatedUnicodeString decode_utf8(Allocator allocator, String utf8, CodeLocation loc = caller_loc()) {

	s64 length = 0;
	for (auto c: utf8) {
		if ((c & 0xC0) == 0x80) {
			continue;
		}
		length += 1;
	}

	Array<char32_t> result = { .allocator = allocator };
	char32_t* data = reserve(&result, length + 1);
	data[length] = '\0';

	auto dst = data;
	auto src = utf8.data;
	while (dst < data + length && src < utf8.data + len(utf8)) {  
		s32 size = 1;
		if        ((src[0] & 0xE0) == 0xC0) {
			size = 2;
		} else if ((src[0] & 0xF0) == 0xE0) {
			size = 3;
		} else if ((src[0] & 0xF8) == 0xF0) {
			size = 4;
		}

		if (src + size > utf8.data + len(utf8)) {
			dst[0] = '?';
			break;
		}

		switch (size) {
			case 1:
				dst[0] = src[0] & 0x8F;
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
			case 4:
				dst[0] =
					char32_t(src[0] & 0x08) << 18 |
					char32_t(src[1] & 0x3F) << 12 |
					char32_t(src[2] & 0x3F) << 6  |
					char32_t(src[3] & 0x3F);
				break;
		}

		dst += 1;
	}
	result.count = length;
	return result;
}

AllocatedUnicodeString decode_utf8(Allocator allocator, const char* c_str, CodeLocation loc = caller_loc()) {
	return decode_utf8(allocator, make_string(c_str), loc);
}

AllocatedUnicodeString decode_utf8(const char* c_str, CodeLocation loc = caller_loc()) {
	return decode_utf8(c_allocator, c_str, loc);
}

AllocatedUnicodeString decode_utf16(Allocator allocator, const char16_t* str, s64 length = -1, Optional<ByteOrder> bo = {}, CodeLocation loc = caller_loc()) {

	if (length < 0) {
		auto ptr = str;
		while (*ptr != '\0') {
			ptr += 1;
		}
		length = ptr - str;
	}

	if (!bo.has_value) {
		if (length > 0 && str[0] == 0xFEFF) {
			bo = BYTE_ORDER_BIG_ENDIAN;
		} else {
			bo = BYTE_ORDER_LITTLE_ENDIAN;
		}
	}
	assert(bo.has_value);

	s64 result_length = 0;
	for (auto i: range(length)) {
		auto c = str[i];
		if (bo != BYTE_ORDER_LITTLE_ENDIAN) {
			swap_endianness(&c);
		}
		if ((c & 0xFC) == 0xD8) {
			continue;
		}
		result_length += 1;
	}

	Array<char32_t> result = { .allocator = allocator };
	char32_t* data = reserve(&result, result_length + 1);
	data[result_length] = '\0';
	result.count = result_length;

	auto dst = data;
	auto src = str;
	while (src < str + length && dst < data + result_length) { 
		auto c = src[0];
		if (bo != BYTE_ORDER_LITTLE_ENDIAN) {
			swap_endianness(&c);
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

AllocatedUnicodeString decode_utf16(const char16_t* str, s64 length = -1, Optional<ByteOrder> bo = {}, CodeLocation loc = caller_loc()) {
	return decode_utf16(c_allocator, str, length, bo, loc);
}
