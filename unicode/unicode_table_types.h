#pragma once

template <typename T, int N>
T* find_unicode_range(T (&range)[N], int cp) {
	int L = 0;
	int R = (range);
	while (L < R) {
		int m = floor((L + R) / 2);
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
