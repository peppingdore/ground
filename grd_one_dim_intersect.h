#pragma once

#include "grd_range.h"

struct GrdOneDimRegion {
	s64 start;
	s64 length;
};

// Regions must not have spaces between them!
void grd_one_dim_patch(s64 regions_count, auto get_region, auto resize, auto insert, auto remove, s64 insert_start, s64 insert_length) {
	if (insert_length < 0) {
		return;
	}

	s64 insert_index = 0;
	
	s64 insert_end = insert_start + insert_length;
	for (s64 i = 0; i < regions_count; i++) {
		GrdOneDimRegion r = get_region(i);
		s64 r_end = r.start + r.length;

		if (insert_start <= r.start && insert_end >= r_end) {
			remove(i);
			i -= 1;
			regions_count -= 1;
			continue;
		}
		if (insert_start < r_end && insert_start >= r.start) {
			if (insert_end < r_end) {
				insert(i + 1, insert_end, r_end, i);
				regions_count += 1;
			}
			if (insert_start == r.start) {
				remove(i);
				i -= 1;
				regions_count -= 1;
			} else {
				resize(i, r.start, insert_start);
			}
			insert_index = i + 1;
			continue;
		}
		if (insert_end > r.start && insert_end <= r_end) {
			if (insert_end == r_end) {
				remove(i);
				i -= 1;
				regions_count -= 1;
			} else {
				resize(i, insert_end, r_end);
			}
		}
	}
	insert(insert_index, insert_start, insert_end, -1);
}
