#pragma once

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
