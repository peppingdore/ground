#pragma once

#include "base.h"
#include "array_view.h"
#include "range.h"

void quick_sort(s64 start, s64 length, auto* arr, auto less, auto swap) {
	if (length == 0) {
		return;
	}

	auto find_greater_on_the_left = [&] (s64 anchor_index) -> s64 {
		for (auto i: range(anchor_index)) {
			if (!less(arr, i, anchor_index)) {
				return i;
			}
		}
		return -1;
	};

	auto find_lower_on_the_right = [&] (s64 anchor_index) -> s64 {
		for (auto i: range_from_to(anchor_index, length)) {
			if (less(arr, i, anchor_index)) {
				return i;
			}
		}
		return -1;
	};

	s64 anchor_index = 0;

	while (true) {
		s64 left_greater = find_greater_on_the_left(anchor_index);
		s64 right_lower  = find_lower_on_the_right(anchor_index);

		if (left_greater == -1 && right_lower == -1) {
			break;
		}

		if (left_greater == -1) {
			swap(arr, anchor_index, right_lower);
			anchor_index = right_lower;
		} else if (right_lower == -1) {
			swap(arr, anchor_index, left_greater);
			anchor_index = left_greater;
		} else {
			swap(arr, left_greater, right_lower);
		}
	}

	quick_sort(0, anchor_index, arr, less, swap);
	quick_sort(anchor_index + 1, length - anchor_index - 1, arr, less, swap);
}

void sort(auto* arr, auto less, auto swap) {
	quick_sort(0, len(*arr), arr, less, swap);
}

void sort(auto* arr, auto less) {
	auto swap = [](auto* arr, s64 a, s64 b) {
		auto temp = *arr->operator[](a);
		*arr->operator[](a) = *arr->operator[](b);
		*arr->operator[](b) = temp;
	};
	sort(arr, less, swap);
}

void sort(auto* arr) {
	auto less = [] (auto* arr, s64 a, s64 b) {
		return *arr->operator[](a) < *arr->operator[](b);
	};
	sort(arr, less);
}

auto sort_key(auto* arr, auto key) {
	auto less = [key](auto* arr, s64 a, s64 b) {
		return key(*arr->operator[](a)) < key(*arr->operator[](b));
	};
	return less;
}

auto sort_reverse(auto less_proc = [](auto* arr, s64 a, s64 b) { 
	return *arr->operator[](a) < *arr->operator[](b);
}) {
	auto less = [less_proc] (s64 a, s64 b) {
		return less_proc(b, a);
	};
	return less;
}
