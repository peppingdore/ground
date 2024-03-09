#pragma once

#include <stdint.h>
#include <float.h>
#include <assert.h>

#ifdef _WIN32
	#define OS_WINDOWS 1
#endif
#ifdef __linux__
	#define OS_LINUX 1
#endif
#ifdef __APPLE__
	#define OS_DARWIN 1
#endif

#define IS_POSIX (OS_LINUX || OS_DARWIN)

#if defined(__clang__)
	#define COMPILER_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
	#define COMPILER_GCC 1
#elif defined(_MSC_VER)
	#define COMPILER_MSVC 1
#else
	#define COMPILER_UNDEFINED 1
	// There's a code that depends on specific compiler.
	static_assert(false);
#endif


#if defined(__x86_64__) || defined(_M_X64)
	#define ARCH_X64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
	#define ARCH_ARM64 1
#endif

// Allows us to pass an argument with commas(,) to a macro without confusing a compiler. 
#define SINGLE_ARG(...) __VA_ARGS__

#if defined(__clang__)
	#define force_inline __attribute__((always_inline))
#elif defined(_MSC_VER)
	#define force_inline __forceinline
#endif

#if COMPILER_MSVC
	#define Debug_Break() __debugbreak()
#else
	#define Debug_Break() __builtin_debugtrap()
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CONCAT_INTERNAL(x,y) x##y
#define CONCAT(x,y) CONCAT_INTERNAL(x,y)

#define OFFSETOF(Type, member) (s64(&((Type*) 0x1000)->member) - s64((Type*) 0x1000))

using s8   = int8_t;
using u8   = uint8_t;
using s16  = int16_t;
using u16  = uint16_t;
using s32  = int32_t;
using u32  = uint32_t;
using s64  = int64_t;
using u64  = uint64_t;
using f32  = float;
using f64  = double;

constexpr s8   s8_max  = 0x7f;
constexpr s8   s8_min  = -s8_max - 1;
constexpr u8   u8_max  = 0xff;
constexpr u8   u8_min  = 0;
constexpr s16  s16_max = 0x7fff;
constexpr s16  s16_min = -s16_max - 1;
constexpr u16  u16_max = 0xffff;
constexpr u16  u16_min = 0;
constexpr s32  s32_max = 0x7fff'ffff;
constexpr s32  s32_min = -s32_max - 1;
constexpr u32  u32_max = 0xffff'ffff;
constexpr u32  u32_min = 0;
constexpr s64  s64_max = 0x7fff'ffff'ffff'ffff;
constexpr s64  s64_min = -s64_max - 1;
constexpr u64  u64_max = 0xffff'ffff'ffff'ffff;
constexpr u64  u64_min = 0;

constexpr s8  s8_sign_mask    = s8 (1 << 7);
constexpr s16 s16_sign_mask   = s16(1 << 15);
constexpr s32 s32_sign_mask   = s32(1 << 31);
constexpr s64 s64_sign_mask   = s64(1LL << 63);

constexpr f32 f32_epsilon = FLT_EPSILON;
constexpr f64 f64_epsilon = DBL_EPSILON;

using Byte_Order = bool;
constexpr Byte_Order BYTE_ORDER_LITTLE_ENDIAN = false;
constexpr Byte_Order BYTE_ORDER_BIG_ENDIAN    = true;

template <typename T, u64 n>
constexpr u64 static_array_count(T(&)[n]) {
	return n;
}