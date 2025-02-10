#pragma once

#include <stdint.h>
#include <float.h>
#include <assert.h>
#include <stddef.h> // size_t

#ifdef _WIN32
	#define GRD_OS_WINDOWS 1
#endif
#ifdef __linux__
	#define GRD_OS_LINUX 1
#endif
#ifdef __APPLE__
	#define GRD_OS_DARWIN 1
#endif

#define GRD_IS_POSIX (GRD_OS_LINUX || GRD_OS_DARWIN)

#if defined(__clang__)
	#define GRD_COMPILER_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
	#define GRD_COMPILER_GCC 1
#elif defined(_MSC_VER)
	#define GRD_COMPILER_MSVC 1
#else
	#define GRD_COMPILER_UNDEFINED 1
	// There's a code that depends on specific compiler.
	static_assert(false);
#endif


#if defined(__x86_64__) || defined(_M_X64)
	#define GRD_ARCH_X64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
	#define GRD_ARCH_ARM64 1
#endif

// Allows us to pass an argument with commas(,) to a macro without confusing a compiler. 
#define GRD_SINGLE_ARG(...) __VA_ARGS__

#if defined(__clang__)
	#define GRD_FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
	#define GRD_FORCE_INLINE __forceinline
#endif

#if GRD_COMPILER_MSVC
	#define GrdDebugBreak() __debugbreak()
#else
	#define GrdDebugBreak() __builtin_debugtrap()
#endif

#define GRD_DEDUP inline
#define GRD_DEF inline auto 

#define GRD_IMPL 1
#if GRD_IMPL
	#define GRD_API(...) extern "C" inline __VA_ARGS__
	#define GRD_API_X(sig, ...) extern "C" inline sig __VA_ARGS__
#else
	#define GRD_API(...) extern "C" __VA_ARGS__ ;
	#define GRD_API_X(sig, ...) extern "C" sig;
#endif

#define GRD_STRINGIFY(x) #x
#define GRD_TOSTRING(x) GRD_STRINGIFY(x)
#define GRD_CONCAT_INTERNAL(x,y) x##y
#define GRD_CONCAT(x,y) GRD_CONCAT_INTERNAL(x,y)

#define GRD_OFFSETOF(Type, member) (s64(&((Type*) 0x1000)->member) - s64((Type*) 0x1000))

#define GRD_NARGS(...) GRD_NARGS_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define GRD_NARGS_(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N

#ifndef s8
    #define s8 int8_t
#endif
#ifndef u8
    #define u8 uint8_t
#endif
#ifndef s16
    #define s16 int16_t
#endif
#ifndef u16
    #define u16 uint16_t
#endif
#ifndef s32
    #define s32 int32_t
#endif
#ifndef u32
    #define u32 uint32_t
#endif
// int64_t is long on Linux, but long long on Windows and Mac.
// If we use int64_t for s64, we can't simply use %lld in printf for both OS's.
// So we hardcode s64 as long long.
// Same applies to u64.
#ifndef s64
	// Make sure that type name is one token.
	//  long long(0) is invalid, where as grd_s64(0) is valid.
	using grd_s64 = long long;
    #define s64 grd_s64
#endif
#ifndef u64
	using grd_u64 = unsigned long long;
    #define u64 grd_u64
#endif
#ifndef f32
    #define f32 float
#endif
#ifndef f64
    #define f64 double
#endif

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

template <typename T, u64 n>
GRD_DEDUP constexpr u64 grd_static_array_count(T(&)[n]) {
	return n;
}

GRD_DEDUP char* const* GRD_ARGV = NULL;
GRD_DEDUP int          GRD_ARGC = 0;

GRD_DEDUP void grd_store_cmdline_args(int argc, char* const* argv) {
	GRD_ARGV = argv;
	GRD_ARGC = argc;
}
