#pragma once

#include "allocator.h"
#include "hash_map.h"
#include "type_utils.h"
#include "stopwatch.h"
#include "defer.h"

struct SubAllocator {
	Allocator                   parent;
	HashMap<void*, EmptyStruct> map;
};

AllocatorProcResult sub_allocator_proc(void* allocator_data, AllocatorProcParams p) {
	auto x = (SubAllocator*) allocator_data;
	switch (p.verb) {
		case ALLOCATOR_VERB_ALLOC: {
			auto res = x->parent.proc(x->parent.data, p);
			put(&x->map, res.data);
			return res;
		}
		break;
		case ALLOCATOR_VERB_REALLOC: {
			auto res = x->parent.proc(x->parent.data, p);
			if (p.old_data != res.data) {
				remove(&x->map, p.old_data);
				put(&x->map, res.data);
			}
			return res;
		}
		break;
		case ALLOCATOR_VERB_FREE: {
			remove(&x->map, p.old_data);
			return x->parent.proc(x->parent.data, p);
		}
		break;
		case ALLOCATOR_VERB_FREE_ALLOCATOR: {
			auto parent = x->parent;
			x->map.free();
			Free(x->parent, x);
			free_allocator(parent);
			return {};
		}
		break;
		default:
			return x->parent.proc(x->parent.data, p);
	}
}

Allocator make_sub_allocator(Allocator parent) {
	auto x = make<SubAllocator>(parent);
	x->parent = parent;
	x->map.allocator = parent;
	return { .proc = sub_allocator_proc, .data = x };
}
