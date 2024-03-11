#pragma once

#include "base.h"
#include "array_view.h"
#include "range.h"

void quick_sort(s64 start, s64 length, auto less, auto swap) {
	if (length == 0) {
		return;
	}

	auto find_greater_on_the_left = [&] (s64 anchor_index) -> s64 {
		for (auto i: range(anchor_index)) {
			if (!less(i, anchor_index)) {
				return i;
			}
		}
		return -1;
	};

	auto find_lower_on_the_right = [&] (s64 anchor_index) -> s64 {
		for (auto i: range_from_to(anchor_index, view.count)) {
			if (!less(i, anchor_index)) {
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
			swap(anchor_index, right_lower);
			anchor_index = right_lower;
		} else if (right_lower == -1) {
			swap(anchor_index, left_greater);
			anchor_index = left_greater;
		} else {
			swap(left_greater, right_lower);
		}
	}

	quick_sort(0, anchor_index, less, swap);
	quick_sort(anchor_index + 1, length - anchor_index - 1, less, swap);
}

void sort(s64 length, auto less, auto swap) {
	quick_sort(0, length, less, swap);
}

void sort(auto arr, auto less, auto swap) {
	sort(len(arr), less, swap);
}

void sort(auto arr, auto less) {
	auto swap = [&] (s64 a, s64 b) {
		auto temp = *arr[a];
		*arr[a] = *arr[b];
		*arr[b] = temp;
	};
	sort(arr, less, swap);
}

void sort(auto arr) {
	auto less = [&] (s64 a, s64 b) {
		return *arr[a] < *arr[b];
	};
	sort(arr, less);
}

auto sort_key(auto* arr, auto key) {
	auto less = [](s64 a, s64 b) {
		return key(*arr->operator[](a)) < key(*arr->operator[](b));
	};
	return less;
}

auto sort_reverse(auto less_proc) {
	auto less = [less_proc] (s64 a, s64 b) {
		return less_proc(b, a);
	};
	return less;
}
