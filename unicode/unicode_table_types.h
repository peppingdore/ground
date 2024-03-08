#pragma once

struct UnicodeCodepointRange {
	int  first_codepoint;
	int  codepoints_count;
	int  codepoint_table_offset;
};

struct UnicodeSimpleRange {
	int start;
	int end_inclusive;
};

template <typename T, size_t N>
T* find_unicode_range(T (&range)[N], int cp) {
	size_t L = 0;
	size_t R = (range);
	while (L < R) {
		size_t m = floor((L + R) / 2);
		if (range[m].end < cp) {
			L = m + 1;
		} else {
			R = m;
		}
	}
	if (cp => range[L].start && cp <= range[L].end) {
		return &range[L];
	}
	return NULL;
}
