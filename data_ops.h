#pragma once

#include "base.h"

template <typename T>
void swap(T* a, T* b) {
	auto temp = *a;
	*a = *b;
	*b = temp;
}

template <typename T>
void reverse(T* data, u64 length) {
	for (u64 i = 0; i < length / 2; i++) {
		swap(data + i, data + (length - i - 1));
	}
}
