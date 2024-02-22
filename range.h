#pragma once

#include "base.h"

template <typename T = s64, bool Ascending = true>
struct Numeric_Range {
	T current;
	T target;

	constexpr void operator++() {
		current += (Ascending ? 1 : -1);
	}

	constexpr void operator--() {
		current += (Ascending ? -1 : 1);
	}

	constexpr operator bool() {
		return target != current;
	}

	constexpr bool operator!=(Numeric_Range other) {
		return (bool) *this;
	}

	constexpr T operator*() {
		if constexpr (Ascending) {
			return current;
		} else {
			return current - 1;
		}
	}

	constexpr Numeric_Range& begin() {
		return *this;
	}

	constexpr Numeric_Range& end() {
		return *this;
	}

	constexpr Numeric_Range<T, !Ascending> reverse() {
		return {
			.current = target,
			.target  = current,
		};
	}
};

template <typename T>
constexpr auto range(T start, T count) {
	return Numeric_Range<T> {
		.current = start,
		.target  = (T) (start + count),
	};
}

template <typename T>
constexpr auto range(T count) {
	return range<T>(0, count);
}

constexpr auto range(s64 start, s64 count) {
	assert(count >= 0);

	Numeric_Range<> r = {
		.current = start,
		.target  = start + count,
	};

	return r;
}

constexpr auto range(s64 count) {
	return range(0, count);
}

constexpr auto range_from_to(s64 start, s64 last_index_exclusive) {
	return range(start, last_index_exclusive - start);
}

template <typename T = s64, bool Ascending = true>
constexpr auto reverse(Numeric_Range<T, Ascending> range) {
	return range.reverse();
}
