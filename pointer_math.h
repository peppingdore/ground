#pragma once

#include "base.h"

template <typename T>
inline T* ptr_add(T* pointer, s64 memory_offset) {
	return (T*) ((u64) pointer + memory_offset);
}

inline s64 ptr_diff(const void* lhs, const void* rhs) {
	u64 lhs_num = (u64) lhs;
	u64 rhs_num = (u64) rhs;
	if (lhs_num >= rhs_num) {
		return (s64) (lhs_num - rhs_num);
	} else {
		return -((s64) (rhs_num - lhs_num));
	}
}
