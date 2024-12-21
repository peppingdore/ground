#pragma once

#include "grd_base.h"

template <typename T>
GRD_DEDUP void grd_swap(T* a, T* b) {
	auto temp = *a;
	*a = *b;
	*b = temp;
}

template <typename T>
GRD_DEDUP void grd_reverse(T* data, u64 length) {
	for (u64 i = 0; i < length / 2; i++) {
		grd_swap(data + i, data + (length - i - 1));
	}
}

template <typename ReturnType, typename T> requires (sizeof(ReturnType) == sizeof(T))
GRD_DEDUP ReturnType grd_bitcast(T thing) {
	return *reinterpret_cast<ReturnType*>(&thing);
}
