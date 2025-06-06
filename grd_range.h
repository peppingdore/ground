#pragma once

#include "grd_base.h"
#include "math/grd_math_base.h"

struct GrdRange {
	s64 start;
	s64 target;

	GrdRange begin() { return *this; }
	GrdRange end() { return *this; }
	void  operator++() { start += grd_sign(target - start); }
	bool  operator!=(GrdRange other) { return start != other.target; }
	s64   operator*() { return start; }
};

GRD_DEDUP GrdRange grd_range_from_to(s64 start, s64 end_exclusive) {
	return { start, grd_max(start, end_exclusive) };
}

GRD_DEDUP GrdRange grd_range(s64 start, s64 count) {
	return { start, grd_max(start, start + count) };
}

GRD_DEDUP GrdRange grd_range(s64 count) {
	return { 0, grd_max(0, count) };
}

GRD_DEDUP GrdRange grd_reverse(GrdRange range) {
	if (range.start < range.target) {
		return { range.target - 1, range.start - 1 };
	} else {
		return { range.target + 1, range.start + 1 };
	}
}
