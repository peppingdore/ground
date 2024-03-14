#pragma once

#include "../base.h"
#include "../data_ops.h"
#include <string.h>

enum class Memory_Order: s32 {
	Relaxed = 1,
	Consume = 2,
	Acquire = 3,
	Release = 4,
	Acq_Rel = 5,
	Seq_Cst = 6,
};

consteval bool is_valid_read_memory_order(Memory_Order mo) {
	return 
		mo == Memory_Order::Relaxed ||
		mo == Memory_Order::Consume ||
		mo == Memory_Order::Acquire ||
		mo == Memory_Order::Seq_Cst;
}

consteval bool is_valid_write_memory_order(Memory_Order mo) {
	return 
		mo == Memory_Order::Relaxed ||
		mo == Memory_Order::Release ||
		mo == Memory_Order::Seq_Cst;
}

template <typename T>
concept Atomic_Size =
	sizeof(T) == 1 || sizeof(T) == 2 ||
	sizeof(T) == 4 || sizeof(T) == 8 || sizeof(T) == 16;

#if !COMPILER_MSVC
	consteval int memory_order_to_gcc(Memory_Order mo) {
		if (mo == Memory_Order::Relaxed) return __ATOMIC_RELAXED;
		if (mo == Memory_Order::Consume) return __ATOMIC_CONSUME;
		if (mo == Memory_Order::Acquire) return __ATOMIC_ACQUIRE;
		if (mo == Memory_Order::Release) return __ATOMIC_RELEASE;
		if (mo == Memory_Order::Acq_Rel) return __ATOMIC_ACQ_REL;
		if (mo == Memory_Order::Seq_Cst) return __ATOMIC_SEQ_CST;
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
	template <>  struct Atomic_Integral_Impl<16> { using Type = struct { s64 low; s64 high; }; };
#else
	template <>  struct Atomic_Integral_Impl<16>{ using Type = __int128_t; };
#endif

template <typename T>
using Atomic_Integral = Atomic_Integral_Impl<sizeof(T)>::Type;


template <Atomic_Size T, Memory_Order mo = Memory_Order::Seq_Cst>
T atomic_exchange(T* dst, T value) {
	using N = Atomic_Integral<T>;

	#if COMPILER_MSVC
		#if ARCH_X64
			#define INTERLOCKED_EXCHANGE(postfix, ...) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);
		#elif ARCH_ARM64
			#define INTERLOCKED_EXCHANGE(postfix)\
			if consteval(mo == Memory_Order::Relaxed) result = _InterlockedExchange##postfix##_nf((N*) dst, __VA_ARGS__);\
			if consteval(mo == Memory_Order::Consume) result = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if consteval(mo == Memory_Order::Acquire) result = _InterlockedExchange##postfix##_acq((N*) dst, __VA_ARGS__);\
			if consteval(mo == Memory_Order::Release) result = _InterlockedExchange##postfix##_rel((N*) dst, __VA_ARGS__);\
			if consteval(mo == Memory_Order::Acq_Rel) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
			if consteval(mo == Memory_Order::Seq_Cst) result = _InterlockedExchange##postfix((N*) dst, __VA_ARGS__);\
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

consteval Memory_Order map_compare_and_swap_mo(Memory_Order success_mo) {
	if (success_mo == Memory_Order::Relaxed) return Memory_Order::Relaxed;
	if (success_mo == Memory_Order::Consume) return Memory_Order::Consume;
	if (success_mo == Memory_Order::Acquire) return Memory_Order::Acquire;
	if (success_mo == Memory_Order::Release) return Memory_Order::Relaxed;
	if (success_mo == Memory_Order::Acq_Rel) return Memory_Order::Acquire;
	else                                     return Memory_Order::Seq_Cst;
}

template <
	Atomic_Size T,
	Memory_Order success_mo = Memory_Order::Seq_Cst,
	Memory_Order failure_mo = map_compare_and_swap_mo(success_mo)
>
T compare_and_swap(T* dst, T comp_v, T xchg_v) {
	using N = Atomic_Integral<T>;

	#if COMPILER_MSVC
		#if ARCH_X64
			#define INTERLOCKED_COMPARE_EXCHANGE(postfix) return _InterlockedCompareExchange##postfix((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));
		#elif ARCH_ARM64
			#define INTERLOCKED_COMPARE_EXCHANGE(postfix)\
			if consteval(success_mo == Memory_Order::Relaxed) return _InterlockedCompareExchange##postfix##_nf((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == Memory_Order::Consume) return _InterlockedCompareExchange##postfix##_acq((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == Memory_Order::Acquire) return _InterlockedCompareExchange##postfix##_acq((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == Memory_Order::Release) return _InterlockedCompareExchange##postfix##_rel((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == Memory_Order::Acq_Rel) return _InterlockedCompareExchange##postfix((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
			if consteval(success_mo == Memory_Order::Seq_Cst) return _InterlockedCompareExchange##postfix((N*) dst, bitcast<N>(xchg_v), bitcast<N>(comp_v));\
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

template <Atomic_Size T, Memory_Order mo = Memory_Order::Seq_Cst>
T atomic_load(T* x) {
	T value = {};
	return compare_and_swap<T, mo, mo>(x, value, value);
}

template <typename T>
bool atomic_internal_compare(T a, T b) {
	return memcmp(&a, &b, sizeof(T)) == 0;
}

template <Atomic_Size T, Memory_Order mo = Memory_Order::Seq_Cst>
void atomic_store(T* dst, T value) {
	while (true) {
		T read = *dst;
		T prev = compare_and_swap<T, mo, mo>(dst, read, value);
		if (atomic_internal_compare(prev, read)) {
			break;
		}
	}
}

template <Atomic_Size T, Memory_Order mo = Memory_Order::Seq_Cst>
T atomic_load_add(T* dst, T add) {
	while (true) {
		T read = *(T*) dst;
		T prev = compare_and_swap<T, mo>(dst, read, read + add);
		if (atomic_internal_compare(prev, read)) {
			return prev;
		}
	}
}
