#pragma once

#include "grd_base.h"
#include "math/grd_basic_functions.h"

struct GrdRange {
	s64 start;
	s64 target;

	GrdRange begin() { return *this; }
	GrdRange end() { return *this; }
	void  operator++() { start += grd_sign(target - start); }
	bool  operator!=(GrdRange other) { return start != other.target; }
	s64   operator*() { return start; }
};

GrdRange range_from_to(s64 start, s64 end_exclusive) {
	return { start, grd_max(start, end_exclusive) };
}

GrdRange grd_range(s64 start, s64 count) {
	return { start, grd_max(start, start + count) };
}

GrdRange grd_range(s64 count) {
	return { 0, grd_max(0, count) };
}

GrdRange grd_reverse(GrdRange range) {
	if (range.start < range.target) {
		return { range.target - 1, range.start - 1 };
	} else {
		return { range.target + 1, range.start + 1 };
	}
}
