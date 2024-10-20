#pragma once

#include "grd_base.h"
#include "grd_span.h"
#include "grd_range.h"

void grd_quick_sort(auto arr, s64 start, s64 end, auto less, auto swap) {
	if ((end - start) < 2) {
		return;
	}
	s64 left = start;
	s64 right = end - 1;
	s64 pivot_index = (left) + (end - left) / 2;
	swap(arr, right, pivot_index);
	
	for (auto i: range_from_to(start, end)) {
		if (less(arr, i, right)) { 
			swap(arr, i, left);
			left += 1;
		}
	}
	swap(arr, left, right);

	grd_quick_sort(arr, start, left, less, swap);
	grd_quick_sort(arr, left + 1, end, less, swap);
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
	for (auto i: grd_range(length - 1)) {
		if (less(arr, i + 1, i)) {
			return false;
		}
	}
	return true;
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
