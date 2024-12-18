#pragma once

#include "grd_base.h"
#include "grd_span.h"
#include "grd_range.h"

GRD_API void grd_quick_sort(void* data, s64 start, s64 end, bool (*less)(void*, s64, s64), void (*swap)(void*, s64, s64)) {
	if ((end - start) < 2) {
		return;
	}
	s64 left = start;
	s64 right = end - 1;
	s64 pivot_index = (left) + (end - left) / 2;
	swap(data, right, pivot_index);
	
	for (auto i: grd_range_from_to(start, end)) {
		if (less(data, i, right)) { 
			swap(data, i, left);
			left += 1;
		}
	}
	swap(data, left, right);

	grd_quick_sort(data, start, left, less, swap);
	grd_quick_sort(data, left + 1, end, less, swap);
}

GRD_API bool grd_is_sorted(void* data, s64 start, s64 length, bool (*less)(void*, s64, s64)) {
	for (auto i: grd_range(length - 1)) {
		if (less(data, i + 1, i)) {
			return false;
		}
	}
	return true;
}

void grd_quick_sort(auto arr, s64 start, s64 end, auto less, auto swap) {
	struct Data {
		decltype(arr)&  arr;
		decltype(less)& less;
		decltype(swap)& swap;
	} data = {
		arr, less, swap
	};

	grd_quick_sort((void*) &data, start, end,
		+[](void* uncasted, s64 a, s64 b) {
			auto data = (Data*)uncasted;
			return data->less(data->arr, a, b);
		},
		+[](void* uncasted, s64 a, s64 b) {
			auto data = (Data*)uncasted;
			data->swap(data->arr, a, b);
		}
	);
}

void grd_sort(auto arr, auto less, auto swap) {
	grd_quick_sort(arr, 0, grd_len(arr), less, swap);
}

void grd_sort(auto arr, auto less) {
	auto swap = [&](auto _, s64 a, s64 b) {
		auto temp = arr[a];
		arr[a] = arr[b];
		arr[b] = temp;
	};
	grd_sort(arr, less, swap);
}

void grd_sort(auto arr) {
	auto less = [&] (auto _, s64 a, s64 b) {
		return arr[a] < arr[b];
	};
	grd_sort(arr, less);
}

bool grd_is_sorted(auto arr, s64 start, s64 length, auto less) {
	struct Data {
		decltype(arr)&  arr;
		decltype(less)& less;
	} data = {
		arr, less
	};

	return grd_is_sorted((void*) &data, start, length,
		+[](void* uncasted, s64 a, s64 b) {
			auto data = (Data*)uncasted;
			return data->less(data->arr, a, b);
		}
	);
}

bool grd_is_sorted(auto arr, auto less) {
	return grd_is_sorted(arr, 0, grd_len(arr), less);
}

bool grd_is_sorted(auto arr) {
	auto less = [&] (auto _, s64 a, s64 b) {
		return arr[a] < arr[b];
	};
	return grd_is_sorted(arr, less);
}
