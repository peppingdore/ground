#pragma once

#include "base.h"

// #if OS_WINDOWS
// 	#define U_STATIC_IMPLEMENTATION
// 	BUILD_RUN("params.add_include_dir(builder.MODULE_ROOT / 'third_party/icu')");
// #endif
// #if OS_DARWIN
// 	BUILD_RUN("params.add_include_dir('/usr/local/opt/icu4c/include')");
// #endif
// #include "third_party/icu/unicode/ustring.h"
// #include "third_party/icu/unicode/uchar.h"
// #include "third_party/icu/unicode/uscript.h"

#include "array_view.h"
#include "tuple.h"
#include "coroutine.h"
#include "optional.h"
#include "byte_order.h"

template <typename T>
concept StringChar = std::same_as<T, char> || std::same_as<T, char32_t>;

// UTF-32 or UTF-8/ASCII.
template <StringChar Char>
struct BaseString {
	Char* data   = NULL;
	s64   length = 0;

	Char operator[](s64 index) {
		assert(index >= 0 && index < length);
		return data[index];
	}

	template <typename T>
	bool operator==(BaseString<T> rhs) {
		if (length != rhs.length) {
			return false;
		}
		if ((void*) data == (void*) rhs.data) {
			return true;
		}

		for (auto i: range(length)) {
			if (data[i] != rhs[i]) {
				return false;
			}
		}
		return true;
	}

	Char* copy_c_str(Allocator allocator = c_allocator) {
		auto cp = allocator.alloc<Char>(length + 1);
		memcpy(cp, data, length * sizeof(Char));
		cp[length] = '\0';
		return cp;
	}

	BaseString<char32_t> copy_UnicodeString(Allocator allocator = c_allocator) 
		requires (std::is_same_v<Char, char>)
	{
		auto mem = allocator.alloc<char32_t>(length);
		for (auto i: range(length)) {
			mem[i] = data[i];
		}
		return { .data = mem, .length = length };
	}

	auto begin() { return data; }
	auto end()   { return data + length; }

	BaseString copy(Allocator allocator = c_allocator, CodeLocation loc = caller_loc()) {
		auto cp = *this;
		cp.data = allocator.alloc<Char>(length, loc);
		memcpy(cp.data, data, length * sizeof(Char));
		return cp;
	}

	void free(Allocator allocator = c_allocator) {
		allocator.free(data);
	}
};

using String         = BaseString<char>;
using UnicodeString = BaseString<char32_t>;

template <StringChar T>
BaseString<T> make_string(const T* data, s64 length) {
	return { .data = (T*) data, .length = length };
}

template <StringChar T>
BaseString<T> make_string(const T* c_str) {
	auto ptr = c_str;
	while (true) {
		if (ptr[0] == '\0') {
			break;
		}
		ptr += 1;
	}
	return { .data = (T*) c_str, .length = pointer_diff(ptr, c_str) };
}

constexpr String operator""_b(const char* c_str, size_t length) {
	return make_string(c_str, length);
}
constexpr UnicodeString operator""_b(const char32_t* c_str, size_t length) {
	return make_string(c_str, length);
}

enum class StringKind: s32 {
	Ascii = 0,
	Utf32 = 1,
};

struct StringType: Type {
	constexpr static auto KIND = make_type_kind("string");

	StringKind string_kind;
};

StringType* reflect_type(String* x, StringType* type) {
	type->name = "String";
	type->string_kind = StringKind::Ascii;
	return type;
}

StringType* reflect_type(UnicodeString* x, StringType* type) {
	type->name = "UnicodeString";
	type->string_kind = StringKind::Utf32;
	return type;
}


s32 utf8_char_size(char32_t c) {
	if (c <= 0x007F) return 1;
	if (c <= 0x07FF) return 2;
	if (c <= 0xFFFF) return 3; 
	                 return 4;
}

bool is_line_break(char32_t c) {
	return
		c == U'\n'     || 
		c == U'\x000C' || // Form Feed
		c == U'\x0085' || // Next Line
		c == U'\x2028' || // Line Separator
		c == U'\x2029';   // Paragraph Separator
}

s64 utf8_length(UnicodeString str) {
	s64 length = 0;
	for (auto i: range(str.length)) {
		length += utf8_char_size(str[i]);
	}
	return length;
}

void encode_utf8(UnicodeString str, char* buf) {
	char* ptr = buf;
	for (auto i: range(str.length)) {
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
String encode_utf8(Allocator allocator, UnicodeString str, CodeLocation loc = caller_loc()) {
	s64 length = utf8_length(str);
	String result = {
		.data   = allocator.alloc<char>(length + 1, loc),
		.length = length
	};
	encode_utf8(str, result.data);
	result.data[length] = '\0';
	return result;
}

String encode_utf8(UnicodeString str, CodeLocation loc = caller_loc()) {
	return encode_utf8(c_allocator, str, loc);
}

s32 utf16_char_size(char32_t c) {
	return c >= 0x10000 ? 2 : 1;
}

// Zero terminated, terminator is at index length, not length - 1.
Tuple<char16_t*, s64> encode_utf16(Allocator allocator, UnicodeString str, CodeLocation loc = caller_loc()) {
	
	s64 length = 0;
	for (auto i: range(str.length)) {
		length += utf16_char_size(str[i]);
	}

	char16_t* data = allocator.alloc<char16_t>(length + 1, loc);
	data[length] = '\0';

	auto ptr = data;
	for (auto i: range(str.length)) {
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

	return make_tuple(data, length);
}

Tuple<char16_t*, s64> encode_utf16(UnicodeString str, CodeLocation loc = caller_loc()) {
	return encode_utf16(c_allocator, str, loc);
}

UnicodeString decode_utf8(Allocator allocator, String utf8, CodeLocation loc = caller_loc()) {

	UnicodeString result;
	for (auto c: utf8) {
		if ((c & 0xC0) == 0x80) {
			continue;
		}
		result.length += 1;
	}

	result.data = allocator.alloc<char32_t>(result.length + 1, loc),
	result.data[result.length] = '\0';

	auto dst = result.data;
	auto src = utf8.data;
	while (dst < result.data + result.length && src < utf8.data + utf8.length) {
		s32 size = 1;
		if        ((src[0] & 0xE0) == 0xC0) {
			size = 2;
		} else if ((src[0] & 0xF0) == 0xE0) {
			size = 3;
		} else if ((src[0] & 0xF8) == 0xF0) {
			size = 4;
		}

		if (src + size > utf8.data + utf8.length) {
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

	return result;
}

UnicodeString decode_utf8(Allocator allocator, const char* c_str, CodeLocation loc = caller_loc()) {
	return decode_utf8(allocator, make_string(c_str), loc);
}

UnicodeString decode_utf8(const char* c_str, CodeLocation loc = caller_loc()) {
	return decode_utf8(c_allocator, c_str, loc);
}

UnicodeString decode_utf16(Allocator allocator, const char16_t* str, s64 length = -1, Optional<ByteOrder> bo = {}, CodeLocation loc = caller_loc()) {

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

	UnicodeString result;
	for (auto i: range(length)) {
		auto c = str[i];
		if (bo != BYTE_ORDER_LITTLE_ENDIAN) {
			swap_endianness(&c);
		}
		if ((c & 0xFC) == 0xD8) {
			continue;
		}
		result.length += 1;
	}

	result.data = allocator.alloc<char32_t>(result.length + 1, loc);

	auto dst = result.data;
	auto src = str;
	while (src < str + length && dst < result.data + result.length) {
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

UnicodeString decode_utf16(const char16_t* str, s64 length = -1, Optional<ByteOrder> bo = {}, CodeLocation loc = caller_loc()) {
	return decode_utf16(c_allocator, str, length, bo, loc);
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
