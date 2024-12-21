#pragma once

#include "../grd_base.h"
#include "../grd_data_ops.h"
#include <type_traits>
#include <string.h>

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
template <>      struct GrdAtomicIntegralImpl<1> { using Type = s8; };
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
		#if ARCH_X64
			#define GRD_INTERLOCKED_EXCHANGE(postfix, ...) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);
		#elif ARCH_ARM64
			#define GRD_INTERLOCKED_EXCHANGE(postfix)\
			if consteval(mo == GrdMemoryOrder::Relaxed) result = _InterlockedExchange##postfix##_nf((N*) dst, __VA_ARGS__);\
			if consteval(mo == GrdMemoryOrder::Consume) result = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if consteval(mo == GrdMemoryOrder::Acquire) result = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if consteval(mo == GrdMemoryOrder::Release) result = _InterlockedExchange##postfix##_rel((N*) dst, __VA_ARGS__);\
			if consteval(mo == GrdMemoryOrder::AcqRel) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
			if consteval(mo == GrdMemoryOrder::SeqCst) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
		#else
			static_assert(false);
		#endif

		N prev;
		if consteval (sizeof(T) == 1) {
			GRD_INTERLOCKED_EXCHANGE(8, grd_bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 2) {
			GRD_INTERLOCKED_EXCHANGE(16, grd_bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 4) {
			GRD_INTERLOCKED_EXCHANGE(, grd_bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 8) {
			GRD_INTERLOCKED_EXCHANGE(64, grd_bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 16) {
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
		#if ARCH_X64
			#define GRD_INTERLOCKED_COMPARE_EXCHANGE(postfix) return _InterlockedCompareExchange##postfix((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));
		#elif ARCH_ARM64
			#define GRD_INTERLOCKED_COMPARE_EXCHANGE(postfix)\
			if consteval(success_mo == GrdMemoryOrder::Relaxed) return _InterlockedCompareExchange##postfix##_nf((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if consteval(success_mo == GrdMemoryOrder::Consume) return _InterlockedCompareExchange##postfix##_acq((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if consteval(success_mo == GrdMemoryOrder::Acquire) return _InterlockedCompareExchange##postfix##_acq((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if consteval(success_mo == GrdMemoryOrder::Release) return _InterlockedCompareExchange##postfix##_rel((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if consteval(success_mo == GrdMemoryOrder::AcqRel) return _InterlockedCompareExchange##postfix((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
			if consteval(success_mo == GrdMemoryOrder::SeqCst) return _InterlockedCompareExchange##postfix((N*) dst, grd_bitcast<N>(xchg_v), grd_bitcast<N>(comp_v));\
		#else
			static_assert(false);
		#endif

		if consteval (sizeof(T) == 1) {
			GRD_INTERLOCKED_COMPARE_EXCHANGE(8);
		} else if consteval (sizeof(T) == 2) {
			GRD_INTERLOCKED_COMPARE_EXCHANGE(16);
		} else if consteval (sizeof(T) == 4) {
			GRD_INTERLOCKED_COMPARE_EXCHANGE();
		} else if consteval (sizeof(T) == 8) {
			GRD_INTERLOCKED_COMPARE_EXCHANGE(64);
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
