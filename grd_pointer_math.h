#pragma once

#include "grd_base.h"

template <typename T>
GRD_DEDUP T* grd_ptr_add(T* pointer, s64 memory_offset) {
	return (T*) ((u64) pointer + memory_offset);
}

GRD_DEDUP s64 grd_ptr_diff(const void* lhs, const void* rhs) {
	u64 lhs_num = (u64) lhs;
	u64 rhs_num = (u64) rhs;
	if (lhs_num >= rhs_num) {
		return (s64) (lhs_num - rhs_num);
	} else {
		return -((s64) (rhs_num - lhs_num));
	}
}
