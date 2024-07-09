#pragma once

#include "../base.h"
#include "../data_ops.h"
#include <string.h>

enum class MemoryOrder: s32 {
	Relaxed = 1,
	Consume = 2,
	Acquire = 3,
	Release = 4,
	Acq_Rel = 5,
	Seq_Cst = 6,
};

consteval bool is_valid_read_memory_order(MemoryOrder mo) {
	return 
		mo == MemoryOrder::Relaxed ||
		mo == MemoryOrder::Consume ||
		mo == MemoryOrder::Acquire ||
		mo == MemoryOrder::Seq_Cst;
}

consteval bool is_valid_write_memory_order(MemoryOrder mo) {
	return 
		mo == MemoryOrder::Relaxed ||
		mo == MemoryOrder::Release ||
		mo == MemoryOrder::Seq_Cst;
}

template <typename T>
concept AtomicSize =
	sizeof(T) == 1 || sizeof(T) == 2 ||
	sizeof(T) == 4 || sizeof(T) == 8 || sizeof(T) == 16;

#if !COMPILER_MSVC
	consteval int memory_order_to_gcc(MemoryOrder mo) {
		if (mo == MemoryOrder::Relaxed) return __ATOMIC_RELAXED;
		if (mo == MemoryOrder::Consume) return __ATOMIC_CONSUME;
		if (mo == MemoryOrder::Acquire) return __ATOMIC_ACQUIRE;
		if (mo == MemoryOrder::Release) return __ATOMIC_RELEASE;
		if (mo == MemoryOrder::Acq_Rel) return __ATOMIC_ACQ_REL;
		if (mo == MemoryOrder::Seq_Cst) return __ATOMIC_SEQ_CST;
		return 0;
	}
#endif

template <int N> struct Atomic_Integral_Impl;
template <>      struct Atomic_Integral_Impl<1> { using Type = s8; };
template <>      struct Atomic_Integral_Impl<2> { using Type = s16; };
#if COMPILER_MSVC
	template <>  struct Atomic_Integral_Impl<4> { using Type = long; };
#else
	template <>  struct Atomic_Integral_Impl<4> { using Type = s32; };
#endif
template <>      struct Atomic_Integral_Impl<8> { using Type = s64; };
#if COMPILER_MSVC
	template <>  struct alignas(16) Atomic_Integral_Impl<16> { using Type = struct { s64 low; s64 high; }; };
#else
	template <>  struct alignas(16) Atomic_Integral_Impl<16>{ using Type = __int128_t; };
#endif

template <typename T>
using Atomic_Integral = Atomic_Integral_Impl<sizeof(T)>::Type;


template <AtomicSize T, MemoryOrder mo = MemoryOrder::Seq_Cst>
T atomic_exchange(T* dst, T value) {
	using N = Atomic_Integral<T>;

	#if COMPILER_MSVC
		#if ARCH_X64
			#define INTERLOCKED_EXCHANGE(postfix, ...) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);
		#elif ARCH_ARM64
			#define INTERLOCKED_EXCHANGE(postfix)\
			if consteval(mo == MemoryOrder::Relaxed) result = _InterlockedExchange##postfix##_nf((N*) dst, __VA_ARGS__);\
			if consteval(mo == MemoryOrder::Consume) result = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if consteval(mo == MemoryOrder::Acquire) result = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if consteval(mo == MemoryOrder::Release) result = _InterlockedExchange##postfix##_rel((N*) dst, __VA_ARGS__);\
			if consteval(mo == MemoryOrder::Acq_Rel) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
			if consteval(mo == MemoryOrder::Seq_Cst) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
		#else
			static_assert(false);
		#endif

		N prev;
		if consteval (sizeof(T) == 1) {
			INTERLOCKED_EXCHANGE(8, bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 2) {
			INTERLOCKED_EXCHANGE(16, bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 4) {
			INTERLOCKED_EXCHANGE(, bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 8) {
			INTERLOCKED_EXCHANGE(64, bitcast<N>(value), *(N*) *dst);
		} else if consteval (sizeof(T) == 16) {
			prev = *(N*)dst;
			INTERLOCKED_EXCHANGE(128, bitcast<N>(value).high, bitcast<N>(value).low, &prev);
		} else {
			static_assert(false);
		}
		return bitcast<T>(prev);
		#undef INTERLOCKED_EXCHANGE

	#else
		N prev_value;
		__atomic_exchange((N*) dst, (N*) &value, &prev_value, memory_order_to_gcc(mo));
		return bitcast<T>(prev_value);
	#endif
}

consteval MemoryOrder map_compare_and_swap_mo(MemoryOrder success_mo) {
	if (success_mo == MemoryOrder::Relaxed) return MemoryOrder::Relaxed;
	if (success_mo == MemoryOrder::Consume) return MemoryOrder::Consume;
	if (success_mo == MemoryOrder::Acquire) return MemoryOrder::Acquire;
	if (success_mo == MemoryOrder::Release) return MemoryOrder::Relaxed;
	if (success_mo == MemoryOrder::Acq_Rel) return MemoryOrder::Acquire;
	else                                    return MemoryOrder::Seq_Cst;
}

template <
	AtomicSize T,
	MemoryOrder success_mo = MemoryOrder::Seq_Cst,
	MemoryOrder failure_mo = map_compare_and_swap_mo(success_mo)
>
T compare_and_swap(T* dst, T comp_v, T xchg_v) {
	using N = Atomic_Integral<T>;

	#if COMPILER_MSVC
		#if ARCH_X64
			#define INTERLOCKED_COMPARE_EXCHANGE(postfix) return _InterlockedCompareExchange##postfix((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));
		#elif ARCH_ARM64
			#define INTERLOCKED_COMPARE_EXCHANGE(postfix)\
			if consteval(success_mo == MemoryOrder::Relaxed) return _InterlockedCompareExchange##postfix##_nf((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == MemoryOrder::Consume) return _InterlockedCompareExchange##postfix##_acq((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == MemoryOrder::Acquire) return _InterlockedCompareExchange##postfix##_acq((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == MemoryOrder::Release) return _InterlockedCompareExchange##postfix##_rel((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == MemoryOrder::Acq_Rel) return _InterlockedCompareExchange##postfix((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == MemoryOrder::Seq_Cst) return _InterlockedCompareExchange##postfix((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
		#else
			static_assert(false);
		#endif

		if consteval (sizeof(T) == 1) {
			INTERLOCKED_COMPARE_EXCHANGE(8);
		} else if consteval (sizeof(T) == 2) {
			INTERLOCKED_COMPARE_EXCHANGE(16);
		} else if consteval (sizeof(T) == 4) {
			INTERLOCKED_COMPARE_EXCHANGE();
		} else if consteval (sizeof(T) == 8) {
			INTERLOCKED_COMPARE_EXCHANGE(64);
		} else {
			static_assert(false);
		}

		#undef INTERLOCKED_COMPARE_EXCHANGE

	#else
		__atomic_compare_exchange((N*) dst, (N*) &comp_v, (N*) &xchg_v, false, memory_order_to_gcc(success_mo), memory_order_to_gcc(failure_mo));
		return bitcast<T>(comp_v);
	#endif
}

template <AtomicSize T, MemoryOrder mo = MemoryOrder::Seq_Cst>
T atomic_load(T* x) {
	T value = {};
	return compare_and_swap<T, mo>(x, value, value);
}

template <typename T>
bool atomic_internal_compare(T a, T b) {
	return memcmp(&a, &b, sizeof(T)) == 0;
}

template <AtomicSize T, MemoryOrder mo = MemoryOrder::Seq_Cst>
void atomic_store(T* dst, T value) {
	while (true) {
		T read = *dst;
		T prev = compare_and_swap<T, mo>(dst, read, value);
		if (atomic_internal_compare(prev, read)) {
			break;
		}
	}
}

template <AtomicSize T, MemoryOrder mo = MemoryOrder::Seq_Cst>
T atomic_load_add(T* dst, T add) {
	while (true) {
		T read = *(T*) dst;
		T prev = compare_and_swap<T, mo>(dst, read, read + add);
		if (atomic_internal_compare(prev, read)) {
			return prev;
		}
	}
}
