#pragma once

#include "base.h"
#include "array_view.h"
#include "range.h"

void quick_sort(auto arr, s64 start, s64 end, auto less, auto swap) {
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

	quick_sort(arr, start, left, less, swap);
	quick_sort(arr, left + 1, end, less, swap);
}

void sort(auto arr, auto less, auto swap) {
	quick_sort(arr, 0, len(arr), less, swap);
}

void sort(auto arr, auto less) {
	auto swap = [&](auto _, s64 a, s64 b) {
		auto temp = *arr[a];
		*arr[a] = *arr[b];
		*arr[b] = temp;
	};
	sort(arr, less, swap);
}

void sort(auto arr) {
	auto less = [&] (auto _, s64 a, s64 b) {
		return *arr[a] < *arr[b];
	};
	sort(arr, less);
}

bool is_sorted(s64 start, s64 length, auto less) {
	for (auto i: range(length - 1)) {
		if (less(i + 1, i)) {
			return false;
		}
	}
	return true;
}

bool is_sorted(auto arr, auto less) {
	return is_sorted(0, len(arr), less);
}

bool is_sorted(auto arr) {
	auto less = [&] (auto _, s64 a, s64 b) {
		return *arr[a] < *arr[b];
	};
	return is_sorted(arr, less);
}
