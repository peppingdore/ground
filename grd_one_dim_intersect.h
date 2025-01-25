#pragma once

#include "grd_range.h"

struct GrdOneDimRegion {
	s64 start;
	s64 end;
};

struct GrdOneDimArrPatcher {
	void*             data = NULL;
	s64             (*get_length)(void*);
	GrdOneDimRegion (*get_region)(void*, s64 index);
	void            (*resize)(void*, s64 index, s64 start, s64 end);
	void            (*insert)(void*, s64 index, s64 start, s64 end, s64 copy_idx);
	void            (*remove)(void*, s64 index);
};

GRD_DEF grd_make_one_dim_patcher(
	void* data,
	s64 (*get_length)(void*),
	GrdOneDimRegion (*get_region)(void*, s64 index),
	void (*resize)(void*, s64 index, s64 start, s64 end),
	void (*insert)(void*, s64 index, s64 start, s64 end, s64 copy_idx),
	void (*remove)(void*, s64 index))
{
	return GrdOneDimArrPatcher {
		.data = data,
		.get_length = get_length,
		.get_region = get_region,
		.resize = resize,
		.insert = insert,
		.remove = remove,
	};
}

// Regions must not have spaces between them!
GRD_DEF grd_one_dim_patch(GrdOneDimArrPatcher* p, s64 insert_start, s64 insert_end) -> s64 {
	if (insert_end <= insert_start) {
		return -1;
	}
	s64 insert_index = 0;
	s64 regions_count = p->get_length(p->data);
	for (s64 i = 0; i < regions_count; i++) {
		GrdOneDimRegion r = p->get_region(p->data, i);

		if (insert_start <= r.start && insert_end >= r.end) {
			p->remove(p->data, i);
			i -= 1;
			regions_count -= 1;
			continue;
		}
		if (insert_start < r.end && insert_start >= r.start) {
			if (insert_end < r.end) {
				p->insert(p->data, i + 1, insert_end, r.end, i);
				regions_count += 1;
			}
			if (insert_start == r.start) {
				p->remove(p->data, i);
				i -= 1;
				regions_count -= 1;
			} else {
				p->resize(p->data, i, r.start, insert_start);
			}
			insert_index = i + 1;
			continue;
		}
		if (insert_end > r.start && insert_end <= r.end) {
			if (insert_end == r.end) {
				p->remove(p->data, i);
				i -= 1;
				regions_count -= 1;
			} else {
				p->resize(p->data, i, insert_end, r.end);
			}
		}
	}
	p->insert(p->data, insert_index, insert_start, insert_end, -1);
	return insert_index;
}

// GRD_DEDUP void grd_one_dim_patch(s64 regions_count, auto get_region, auto resize, auto insert, auto remove, s64 insert_start, s64 insert_length) {
// 	if (insert_length < 0) {
// 		return;
// 	}

// 	s64 insert_index = 0;
	
// 	s64 insert_end = insert_start + insert_length;
// 	for (s64 i = 0; i < regions_count; i++) {
// 		GrdOneDimRegion r = get_region(i);

// 		if (insert_start <= r.start && insert_end >= r.end) {
// 			remove(i);
// 			i -= 1;
// 			regions_count -= 1;
// 			continue;
// 		}
// 		if (insert_start < r.end && insert_start >= r.start) {
// 			if (insert_end < r.end) {
// 				insert(i + 1, insert_end, r.end, i);
// 				regions_count += 1;
// 			}
// 			if (insert_start == r.start) {
// 				remove(i);
// 				i -= 1;
// 				regions_count -= 1;
// 			} else {
// 				resize(i, r.start, insert_start);
// 			}
// 			insert_index = i + 1;
// 			continue;
// 		}
// 		if (insert_end > r.start && insert_end <= r.end) {
// 			if (insert_end == r.end) {
// 				remove(i);
// 				i -= 1;
// 				regions_count -= 1;
// 			} else {
// 				resize(i, insert_end, r.end);
// 			}
// 		}
// 	}
// 	insert(insert_index, insert_start, insert_end, -1);
// }
