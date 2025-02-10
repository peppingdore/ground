#pragma once

#include "grd_base.h"
#include "grd_range.h"
#include "grd_code_location.h"
#include "math/grd_math_base.h"
#include "grd_reflect.h"

#include <cstdlib>
#include <new>
#include <stdio.h>

enum GrdAllocatorVerb: s32 {
	GRD_ALLOCATOR_VERB_ALLOC          = 1 << 0,
	GRD_ALLOCATOR_VERB_FREE           = 1 << 2,
	GRD_ALLOCATOR_VERB_REALLOC        = 1 << 1,
	GRD_ALLOCATOR_VERB_GET_TYPE       = 1 << 3,
	GRD_ALLOCATOR_VERB_FREE_ALLOCATOR = 1 << 4,
};

struct GrdAllocatorProcResult {
	void*    data = NULL;
	GrdType* allocator_type = NULL;
};

struct GrdAllocatorProcParams {
    GrdAllocatorVerb verb;
	void*            old_data = 0;
	u64              old_size = 0;
	u64              new_size = 0;
	GrdCodeLoc       loc;
};

using GrdAllocatorProc = GrdAllocatorProcResult (void* allocator_data, GrdAllocatorProcParams);

struct GrdAllocator {
	// We could just put |proc| in AllocatorData struct,
	//   but that wold mean having level of indirection 2,
	//   which I don't like.
	GrdAllocatorProc* proc = NULL;
	void*             data = NULL;

	bool operator==(GrdAllocator rhs) {
		return proc == rhs.proc && data == rhs.data;
	}
};

GRD_DEDUP GrdType* grd_get_allocator_type(GrdAllocator allocator) {
	return (GrdType*) allocator.proc(allocator.data, {.verb = GRD_ALLOCATOR_VERB_GET_TYPE, .loc = grd_current_loc()}).allocator_type;	
}

GRD_DEDUP void grd_free_allocator(GrdAllocator allocator) {
	allocator.proc(allocator.data, {.verb = GRD_ALLOCATOR_VERB_FREE_ALLOCATOR, .loc = grd_current_loc()});
}


GRD_DEDUP void* grd_malloc_crash_on_failure(u64 size) {
	void* result = malloc(size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to malloc(%zx)", (size_t) size);
	GrdDebugBreak();
	exit(-1);
}
GRD_DEDUP void* grd_realloc_crash_on_failure(void* data, u64 size) {
	void* result = realloc(data, size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to realloc(%p, %zx)", data, (size_t) size);
	GrdDebugBreak();
	exit(-1);
}

struct GrdCrtAllocator {
	GRD_REFLECT(GrdCrtAllocator) {}
};

GRD_DEDUP GrdAllocatorProcResult grd_c_allocator_proc(void* allocator_data, GrdAllocatorProcParams params) {
	switch (params.verb) {
		case GRD_ALLOCATOR_VERB_ALLOC:
			return { .data = grd_malloc_crash_on_failure(params.new_size) };
		case GRD_ALLOCATOR_VERB_REALLOC:
			return { .data = grd_realloc_crash_on_failure(params.old_data, params.new_size) };
		case GRD_ALLOCATOR_VERB_FREE:
			free(params.old_data);
			break;
		case GRD_ALLOCATOR_VERB_GET_TYPE:
			return { .allocator_type = grd_reflect_type_of<GrdCrtAllocator>() };
		default:
			assert(false);
			return {};
	}
	return {};
}

GRD_DEDUP constexpr GrdAllocator grd_crt_allocator = {
	.proc = &grd_c_allocator_proc,
	.data = NULL,
};

// Aliasing like this allows us to replace c_allocator
//   with other allocator in runtime.
// @TODO: rename c_allocator
// @TODO: rename null_allocator
GRD_DEDUP GrdAllocator c_allocator = grd_crt_allocator;

GRD_DEDUP constexpr GrdAllocator null_allocator = { 
	.proc = NULL,
	.data = NULL,
};

GRD_DEDUP void* GrdMalloc(GrdAllocator allocator, u64 size, GrdCodeLoc loc = grd_caller_loc()) {
	GrdAllocatorProcParams p = {
		.verb = GRD_ALLOCATOR_VERB_ALLOC,
		.new_size = size,
		.loc = loc
	};
	return allocator.proc(allocator.data, p).data;
}

GRD_DEDUP void* GrdMalloc(u64 size, GrdCodeLoc loc = grd_caller_loc()) {
	return GrdMalloc(c_allocator, size, loc);
}

GRD_DEDUP void GrdFree(GrdAllocator allocator, void* data, GrdCodeLoc loc = grd_caller_loc()) {
	if (data == NULL) {
		return;
	}
	GrdAllocatorProcParams p = {
		.verb = GRD_ALLOCATOR_VERB_FREE,
		.old_data = data,
		.loc = loc
	};
	allocator.proc(allocator.data, p);
}

GRD_DEDUP void GrdFree(void* data, GrdCodeLoc loc = grd_caller_loc()) {
	GrdFree(c_allocator, data, loc);
}

GRD_DEDUP void* GrdRealloc(GrdAllocator allocator, void* data, u64 old_size, u64 new_size, GrdCodeLoc loc = grd_caller_loc()) {
	GrdAllocatorProcParams p = {
		.verb = GRD_ALLOCATOR_VERB_REALLOC,
		.old_data = data,
		.old_size = old_size,
		.new_size = new_size,
		.loc = loc
	};
	return allocator.proc(allocator.data, p).data;
}

GRD_DEDUP void* GrdRealloc(void* data, u64 old_size, u64 new_size, GrdCodeLoc loc = grd_caller_loc()) {
	return GrdRealloc(c_allocator, data, old_size, new_size, loc);
}

template <typename T>
GRD_DEDUP T* GrdAlloc(GrdAllocator allocator, u64 count, GrdCodeLoc loc = grd_caller_loc()) {
	return (T*) GrdMalloc(allocator, sizeof(T) * count, loc);
}

template <typename T>
GRD_DEDUP T* GrdAlloc(u64 count, GrdCodeLoc loc = grd_caller_loc()) {
	return GrdAlloc<T>(c_allocator, count, loc);
}

template <typename T>
GRD_DEDUP T* grd_copy_value(GrdAllocator allocator, T thing, GrdCodeLoc loc = grd_caller_loc()) {
	T* mem = GrdAlloc<T>(1, loc);
	memcpy(mem, &thing, sizeof(T));
	return mem;
}

template <typename T>
GRD_DEDUP T* grd_make(GrdAllocator allocator = c_allocator, GrdCodeLoc loc = grd_caller_loc()) {
	T* mem = GrdAlloc<T>(allocator, 1, loc);
	return new(mem) T();
}

template <typename T>
GRD_DEDUP T* grd_make(s64 count, GrdAllocator allocator = c_allocator, GrdCodeLoc loc = grd_caller_loc()) {
	T* mem = (T*) GrdAlloc<T>(allocator, count, loc); 
	for (auto i: grd_range(count)) {
		new(mem + i) T();
	}
	return mem;
}
