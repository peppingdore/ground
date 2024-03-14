#pragma once

#include "base.h"
#include "hash.h"
#include "allocator.h"
#include "pointer_math.h"

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

	BaseString<char32_t> copy_unicode_string(Allocator allocator = c_allocator) 
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

using String        = BaseString<char>;
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
	return { .data = (T*) c_str, .length = ptr_diff(ptr, c_str) };
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

template <typename T>
void type_hash(Hasher* hasher, BaseString<T> string) {
	hasher->hash(string.data, sizeof(T) * string.length);
}

s64 len(String str) {
	return str.length;
}

s64 len(UnicodeString str) {
	return str.length;
}
