#pragma once

#include "../grd_base.h"
#include "../grd_data_ops.h"
#include <type_traits>
#include <string.h>
#if GRD_COMPILER_MSVC
	#include <intrin.h>
#endif

enum class GrdMemoryOrder: s32 {
	Relaxed = 1,
	Consume = 2,
	Acquire = 3,
	Release = 4,
	AcqRel  = 5,
	SeqCst  = 6,
};

template <typename T>
concept GrdAtomicSize =
	sizeof(T) == 1 || sizeof(T) == 2 ||
	sizeof(T) == 4 || sizeof(T) == 8 || sizeof(T) == 16;

#if !GRD_COMPILER_MSVC
	GRD_DEDUP consteval int grd_memory_order_to_gcc(GrdMemoryOrder mo) {
		if (mo == GrdMemoryOrder::Relaxed) return __ATOMIC_RELAXED;
		if (mo == GrdMemoryOrder::Consume) return __ATOMIC_CONSUME;
		if (mo == GrdMemoryOrder::Acquire) return __ATOMIC_ACQUIRE;
		if (mo == GrdMemoryOrder::Release) return __ATOMIC_RELEASE;
		if (mo == GrdMemoryOrder::AcqRel)  return __ATOMIC_ACQ_REL;
		if (mo == GrdMemoryOrder::SeqCst)  return __ATOMIC_SEQ_CST;
		return 0;
	}
#endif

template <int N> struct GrdAtomicIntegralImpl;
#if GRD_COMPILER_MSVC
	template <>      struct GrdAtomicIntegralImpl<1> { using Type = char; };
#else
	template <>      struct GrdAtomicIntegralImpl<1> { using Type = s8; };
#endif
template <>      struct GrdAtomicIntegralImpl<2> { using Type = s16; };
#if GRD_COMPILER_MSVC
	template <>  struct GrdAtomicIntegralImpl<4> { using Type = long; };
#else
	template <>  struct GrdAtomicIntegralImpl<4> { using Type = s32; };
#endif
template <>      struct GrdAtomicIntegralImpl<8> { using Type = s64; };
#if GRD_COMPILER_MSVC
	template <>  struct alignas(16) GrdAtomicIntegralImpl<16> { using Type = struct { s64 low; s64 high; }; };
#else
	template <>  struct alignas(16) GrdAtomicIntegralImpl<16>{ using Type = __int128_t; };
#endif

template <typename T>
using GrdAtomicIntegral = GrdAtomicIntegralImpl<sizeof(T)>::Type;


template <GrdAtomicSize T, GrdMemoryOrder mo = GrdMemoryOrder::SeqCst>
GRD_DEDUP T grd_atomic_exchange(T* dst, std::type_identity_t<T> value) {
	using N = GrdAtomicIntegral<T>;

	#if GRD_COMPILER_MSVC
		#if GRD_ARCH_X64
			#define GRD_INTERLOCKED_EXCHANGE(postfix, ...) prev = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);
		#elif GRD_ARCH_ARM64
			#define GRD_INTERLOCKED_EXCHANGE(postfix, ...)\
			if constexpr (mo == GrdMemoryOrder::Relaxed) prev = _InterlockedExchange##postfix##_nf((N*) dst, __VA_ARGS__);\
			if constexpr (mo == GrdMemoryOrder::Consume) prev = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if constexpr (mo == GrdMemoryOrder::Acquire) prev = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if constexpr (mo == GrdMemoryOrder::Release) prev = _InterlockedExchange##postfix##_rel((N*) dst, __VA_ARGS__);\
			if constexpr (mo == GrdMemoryOrder::AcqRel) prev = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
			if constexpr (mo == GrdMemoryOrder::SeqCst) prev = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
		#else
			static_assert(false);
		#endif

		N prev;
		if constexpr (sizeof(T) == 1) {
			GRD_INTERLOCKED_EXCHANGE(8, (N*) dst, grd_bitcast<N>(value));
		} else if constexpr (sizeof(T) == 2) {
			GRD_INTERLOCKED_EXCHANGE(16, (N*) dst, grd_bitcast<N>(value));
		} else if constexpr (sizeof(T) == 4) {
			GRD_INTERLOCKED_EXCHANGE(, (N*) dst, grd_bitcast<N>(value));
		} else if constexpr (sizeof(T) == 8) {
			GRD_INTERLOCKED_EXCHANGE(64, (N*) dst, grd_bitcast<N>(value));
		} else if constexpr (sizeof(T) == 16) {
			// @TODO: wtf is this??
			//  There is no documented _InterlockedExchange128 in the Internet. WTF.
			prev = *(N*)dst;
			GRD_INTERLOCKED_EXCHANGE(128, grd_bitcast<N>(value).high, grd_bitcast<N>(value).low, &prev);
		} else {
			static_assert(false);
		}
		return grd_bitcast<T>(prev);
		#undef GRD_INTERLOCKED_EXCHANGE

	#else
		N prev_value;
		__atomic_exchange((N*) dst, (N*) &value, &prev_value, grd_memory_order_to_gcc(mo));
		return grd_bitcast<T>(prev_value);
	#endif
}

GRD_DEDUP consteval GrdMemoryOrder grd_map_compare_and_swap_mo(GrdMemoryOrder success_mo) {
	if (success_mo == GrdMemoryOrder::Relaxed) return GrdMemoryOrder::Relaxed;
	if (success_mo == GrdMemoryOrder::Consume) return GrdMemoryOrder::Consume;
	if (success_mo == GrdMemoryOrder::Acquire) return GrdMemoryOrder::Acquire;
	if (success_mo == GrdMemoryOrder::Release) return GrdMemoryOrder::Relaxed;
	if (success_mo == GrdMemoryOrder::AcqRel)  return GrdMemoryOrder::Acquire;
	else                                       return GrdMemoryOrder::SeqCst;
}

template <
	GrdAtomicSize T,
	GrdMemoryOrder success_mo = GrdMemoryOrder::SeqCst,
	GrdMemoryOrder failure_mo = grd_map_compare_and_swap_mo(success_mo)
>
GRD_DEDUP T grd_compare_and_swap(T* dst, T comp_v, T xchg_v) {
	using N = GrdAtomicIntegral<T>;

	#if GRD_COMPILER_MSVC
		#if GRD_ARCH_X64
			#define GRD_INTERLOCKED_COMPARE_EXCHANGE(postfix) _InterlockedCompareExchange##postfix((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));
		#elif GRD_ARCH_ARM64
			#define GRD_INTERLOCKED_COMPARE_EXCHANGE(postfix)\
			if constexpr(success_mo == GrdMemoryOrder::Relaxed) _InterlockedCompareExchange##postfix##_nf((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if constexpr(success_mo == GrdMemoryOrder::Consume) _InterlockedCompareExchange##postfix##_acq((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if constexpr(success_mo == GrdMemoryOrder::Acquire) _InterlockedCompareExchange##postfix##_acq((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if constexpr(success_mo == GrdMemoryOrder::Release) _InterlockedCompareExchange##postfix##_rel((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if constexpr(success_mo == GrdMemoryOrder::AcqRel) _InterlockedCompareExchange##postfix((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if constexpr(success_mo == GrdMemoryOrder::SeqCst) _InterlockedCompareExchange##postfix((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
		#else
			static_assert(false);
		#endif

		if constexpr (sizeof(T) == 1) {
			auto prev = GRD_INTERLOCKED_COMPARE_EXCHANGE(8);
			return grd_bitcast<T>(prev);
		} else if constexpr (sizeof(T) == 2) {
			auto prev = GRD_INTERLOCKED_COMPARE_EXCHANGE(16);
			return grd_bitcast<T>(prev);
		} else if constexpr (sizeof(T) == 4) {
			auto prev = GRD_INTERLOCKED_COMPARE_EXCHANGE();
			return grd_bitcast<T>(prev);
		} else if constexpr (sizeof(T) == 8) {
			auto prev = GRD_INTERLOCKED_COMPARE_EXCHANGE(64);
			return grd_bitcast<T>(prev);
		} else {
			static_assert(false);
		}

		#undef GRD_INTERLOCKED_COMPARE_EXCHANGE

	#else
		__atomic_compare_exchange((N*) dst, (N*) &comp_v, (N*) &xchg_v, false, grd_memory_order_to_gcc(success_mo), grd_memory_order_to_gcc(failure_mo));
		return grd_bitcast<T>(comp_v);
	#endif
}

template <GrdAtomicSize T, GrdMemoryOrder mo = GrdMemoryOrder::SeqCst>
GRD_DEDUP T grd_atomic_load(T* x) {
	T value = {};
	return grd_compare_and_swap<T, mo>(x, value, value);
}

template <typename T>
GRD_DEDUP bool grd_atomic_internal_compare(T a, std::type_identity_t<T> b) {
	return memcmp(&a, &b, sizeof(T)) == 0;
}

template <GrdAtomicSize T, GrdMemoryOrder mo = GrdMemoryOrder::SeqCst>
GRD_DEDUP void grd_atomic_store(T* dst, std::type_identity_t<T> value) {
	while (true) {
		T read = *dst;
		T prev = grd_compare_and_swap<T, mo>(dst, read, value);
		if (grd_atomic_internal_compare(prev, read)) {
			break;
		}
	}
}

template <GrdAtomicSize T, GrdMemoryOrder mo = GrdMemoryOrder::SeqCst>
GRD_DEDUP T grd_atomic_load_add(T* dst, std::type_identity_t<T> add) {
	while (true) {
		T read = *(T*) dst;
		T prev = grd_compare_and_swap<T, mo>(dst, read, read + add);
		if (grd_atomic_internal_compare(prev, read)) {
			return prev;
		}
	}
}
