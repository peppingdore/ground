#pragma once

#include "grd_allocator.h"
#include "grd_hash_map.h"
#include "grd_type_utils.h"
#include "grd_stopwatch.h"
#include "grd_defer.h"

struct GrdSubAllocator {
	GrdAllocator                      parent;
	GrdHashMap<void*, GrdEmptyStruct> map;
};

GrdAllocatorProcResult grd_sub_allocator_proc(void* allocator_data, GrdAllocatorProcParams p) {
	auto x = (GrdSubAllocator*) allocator_data;
	switch (p.verb) {
		case GRD_ALLOCATOR_VERB_ALLOC: {
			auto res = x->parent.proc(x->parent.data, p);
			grd_put(&x->map, res.data);
			return res;
		}
		break;
		case GRD_ALLOCATOR_VERB_REALLOC: {
			auto res = x->parent.proc(x->parent.data, p);
			if (p.old_data != res.data) {
				grd_remove(&x->map, p.old_data);
				grd_put(&x->map, res.data);
			}
			return res;
		}
		break;
		case GRD_ALLOCATOR_VERB_FREE: {
			grd_remove(&x->map, p.old_data);
			return x->parent.proc(x->parent.data, p);
		}
		break;
		case GRD_ALLOCATOR_VERB_FREE_ALLOCATOR: {
			auto parent = x->parent;
			x->map.free();
			GrdFree(x->parent, x);
			grd_free_allocator(parent);
			return {};
		}
		break;
		default:
			return x->parent.proc(x->parent.data, p);
	}
}

GrdAllocator grd_make_sub_allocator(GrdAllocator parent) {
	auto x = grd_make<GrdSubAllocator>(parent);
	x->parent = parent;
	x->map.allocator = parent;
	return { .proc = grd_sub_allocator_proc, .data = x };
}
