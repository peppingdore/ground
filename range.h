#pragma once

#include "base.h"
#include "math/basic_functions.h"

struct Range {
	s64 start;
	s64 target;

	Range begin() { return *this; }
	Range end() { return *this; }
	void  operator++() { start += sign(target - start); }
	bool  operator!=(Range other) { return start != other.target; }
	s64   operator*() { return start; }
};

Range range(s64 start, s64 count) {
	return { start, start + count };
}

Range range(s64 count) {
	return { 0, count };
}

Range range_from_to(s64 start, s64 end_exclusive) {
	return { start, end_exclusive };
}

Range reverse(Range range) {
	if (range.start < range.target) {
		return { range.target - 1, range.start - 1 };
	} else {
		return { range.target + 1, range.start + 1 };
	}
}
