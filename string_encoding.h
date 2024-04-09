#pragma once

#include "string_def.h"
#include "optional.h"
#include "byte_order.h"
#include "tuple.h"
#include "code_location.h"

s32 utf8_char_size(char32_t c) {
	if (c <= 0x007F) return 1;
	if (c <= 0x07FF) return 2;
	if (c <= 0xFFFF) return 3; 
	                 return 4;
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
		.data   = Alloc<char>(allocator, length + 1, loc),
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

	char16_t* data = Alloc<char16_t>(allocator, length + 1, loc);
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

	result.data = Alloc<char32_t>(allocator, result.length + 1, loc);
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

	result.data = Alloc<char32_t>(allocator, result.length + 1, loc);

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