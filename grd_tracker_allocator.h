#pragma once

#include "grd_hash_map.h" 
#include "grd_arena_allocator.h"
#include "grd_sort.h"
#include "grd_log.h"
#include "sync/grd_mutex.h"

struct GrdTrackedAlloc {
	u64        size;
	GrdCodeLoc initial_loc;
	GrdCodeLoc loc;
};

struct GrdTrackerAllocator {
	GrdAllocator                       parent_allocator;
	GrdMutex                           mutex;
	GrdHashMap<void*, GrdTrackedAlloc> allocations = { .allocator = null_allocator };
	GrdHashMap<const char*,   u64>     memory_usage_by_file = { .allocator = null_allocator };
	GrdHashMap<GrdCodeLoc, u64>        memory_usage_by_location = { .allocator = null_allocator }; 
	u64                                memory_usage = 0;

	// Hook's results are ignored, just do 'return {}'.
	GrdAllocatorProc*                  pre_hook = NULL;
	GrdAllocatorProc*                  post_hook = NULL;

	GRD_REFLECT(GrdTrackerAllocator) {

	}
};

GRD_DEF grd_get_tracker_allocator(GrdAllocator allocator) -> GrdTrackerAllocator* {
	auto tp = grd_get_allocator_type(allocator);
	if (tp == grd_reflect_type_of<GrdTrackerAllocator>()) {
		return (GrdTrackerAllocator*) allocator.data;
	}
	return NULL;
}

GRD_DEF grd_tracker_allocator_print_usage(GrdAllocator x) -> void {
	auto ta = grd_get_tracker_allocator(x);
	if (!ta) {
		return;
	}

	auto arena = grd_make_arena_allocator(ta->parent_allocator, 1 * 1024 * 1024);
	grd_defer_x(grd_free_allocator(arena));

	GrdArray<decltype(ta->memory_usage_by_file)::Entry> by_file;
	by_file.allocator = arena;

	for (auto e: ta->memory_usage_by_file.iterate()) {
		grd_add(&by_file, *e);
	}

	GrdArray<decltype(ta->memory_usage_by_location)::Entry> by_location;
	by_location.allocator = arena;

	for (auto e: ta->memory_usage_by_location.iterate()) {
		grd_add(&by_location, *e);
	}

	grd_sort(by_file,     [](auto& arr, auto a, auto b) { return arr[a].value < arr[b].value; });
	grd_sort(by_location, [](auto& arr, auto a, auto b) { return arr[a].value < arr[b].value; });
	grd_reverse(by_file);
	grd_reverse(by_location);

	u64 total = 0;
	for (auto i: by_file) {
		total += i.value;
	}

	printf("Total: %zu\n", (size_t) total);

	printf("\n");
	printf("\n");
	printf("By file\n");

	for (auto e: by_file) {
		printf("%zu - %s\n", (size_t) e.value, e.key);
	}

	printf("\n");
	printf("\n");
	printf("By location\n");

	for (auto& e: by_location) {
		printf("%zu - %s, %d\n", (size_t) e.value, e.key.file, e.key.line);
	}
}

GRD_DEDUP GrdAllocatorProcResult grd_tracker_allocator_proc(void* allocator_data, GrdAllocatorProcParams params) {
	auto* ta = (GrdTrackerAllocator*) allocator_data;
	GrdScopedLock(ta->mutex);

	if (ta->pre_hook) {
		ta->pre_hook(ta, params);
	}

	grd_defer {
		if (ta->post_hook) {
			ta->post_hook(ta, params);
		}
	};

	switch (params.verb) {
		case GRD_ALLOCATOR_VERB_ALLOC: {
			auto result = ta->parent_allocator.proc(ta->parent_allocator.data, params);
			grd_put(&ta->allocations, result.data, GrdTrackedAlloc {
				.size = params.new_size,
				.initial_loc = params.loc,
				.loc = params.loc
			});

			u64* used = grd_get(&ta->memory_usage_by_file, params.loc.file);
			if (!used) {
				grd_put(&ta->memory_usage_by_file, params.loc.file, params.new_size);
			} else {
				*used += params.new_size;
			}

			used = grd_get(&ta->memory_usage_by_location, params.loc);
			if (!used) {
				grd_put(&ta->memory_usage_by_location, params.loc, params.new_size);
			} else {
				*used += params.new_size;
			}
			ta->memory_usage += params.new_size;
			return result;
		}
		break;
		case GRD_ALLOCATOR_VERB_REALLOC: {
			GrdTrackedAlloc found_allocation = *grd_get(&ta->allocations, params.old_data);
			grd_remove(&ta->allocations, params.old_data);

			auto new_alloc = ta->parent_allocator.proc(ta->parent_allocator.data, params);

			// Underflow because old_size > new_size is fine.
			u64 diff = params.new_size - params.old_size;
			ta->memory_usage += diff;

			u64* used = grd_get(&ta->memory_usage_by_file, found_allocation.initial_loc.file);
			*used += diff;

			used = grd_get(&ta->memory_usage_by_location, found_allocation.initial_loc);
			*used += diff;

			grd_put(&ta->allocations, new_alloc.data, GrdTrackedAlloc {
				.size = params.new_size,
				.initial_loc = found_allocation.initial_loc,
				.loc = params.loc
			});
			return new_alloc;
		}
		break;
		case GRD_ALLOCATOR_VERB_FREE: {
			auto found_alloc = *grd_get(&ta->allocations, params.old_data);
			ta->memory_usage -= found_alloc.size;
			u64* used = grd_get(&ta->memory_usage_by_file, found_alloc.initial_loc.file);
			*used -= found_alloc.size;
			used = grd_get(&ta->memory_usage_by_location, found_alloc.initial_loc);
			*used -= found_alloc.size;
			grd_remove(&ta->allocations, params.old_data);

			return ta->parent_allocator.proc(ta->parent_allocator.data, params);
		}
		break;

		case GRD_ALLOCATOR_VERB_FREE_ALLOCATOR: {
			grd_lock(&ta->mutex);
			ta->allocations.free();
			ta->memory_usage_by_file.free();
			ta->memory_usage_by_location.free();
			grd_unlock(&ta->mutex);
			ta->mutex.free();
			return {};
		}
		break;

		case GRD_ALLOCATOR_VERB_GET_TYPE:
			return { .allocator_type = grd_reflect_type_of<GrdTrackerAllocator>() };

		default:
			assert(false);
			return {};
	}
}

GRD_DEF grd_make_tracker_allocator(GrdAllocator parent_allocator = c_allocator) -> GrdAllocator {
	auto ta = grd_make<GrdTrackerAllocator>();
	ta->parent_allocator = parent_allocator;
	ta->memory_usage_by_file.allocator = parent_allocator;
	ta->memory_usage_by_location.allocator = parent_allocator;
	ta->allocations.allocator = parent_allocator;
	grd_make_mutex(&ta->mutex);

	return {
		.proc = grd_tracker_allocator_proc,
		.data = ta,
	};
}

GRD_DEF grd_tracker_allocator_is_empty(GrdAllocator x) -> bool {
	auto ta = grd_get_tracker_allocator(x);
	if (!ta) {
		return true;
	}
	GrdScopedLock(ta->mutex);
	return grd_len(ta->allocations) == 0;
}
