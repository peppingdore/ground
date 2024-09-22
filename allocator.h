#pragma once

#include "base.h"
#include "range.h"
#include "code_location.h"
#include "math/basic_functions.h"
#include "reflect.h"

#include <cstdlib>
#include <new>
#include <stdio.h>

enum AllocatorVerb: s32 {
	ALLOCATOR_VERB_NONE           = 0,
	ALLOCATOR_VERB_ALLOC          = 1 << 0,
	ALLOCATOR_VERB_FREE           = 1 << 2,
	ALLOCATOR_VERB_REALLOC        = 1 << 1,
	ALLOCATOR_VERB_GET_TYPE       = 1 << 3,
	ALLOCATOR_VERB_FREE_ALLOCATOR = 1 << 4,
};

struct AllocatorProcResult {
	void* data = NULL;
	Type* allocator_type = NULL;
};

struct AllocatorProcParams {
	AllocatorVerb verb;
	void*         old_data = 0;
	u64           old_size = 0;
	u64           new_size = 0;
	void*         allocator_data = 0;
	CodeLocation  loc;
};

using AllocatorProc = AllocatorProcResult (AllocatorProcParams);

struct Allocator {
	// We could just put |proc| in AllocatorData struct,
	//   but that wold mean having level of indirection 2,
	//   which I don't like.
	AllocatorProc* proc;
	void*          allocator_data;

	bool operator==(Allocator rhs) {
		return proc == rhs.proc && allocator_data == rhs.allocator_data;
	}
};

Type* get_allocator_type(Allocator allocator) {
	return allocator.proc({.verb = ALLOCATOR_VERB_GET_TYPE, .loc = current_loc()}).type;	
}

void free_allocator(Allocator allocator) {
	allocator.proc({.verb = ALLOCATOR_VERB_FREE_ALLOCATOR, .loc = current_loc()});
}


void* malloc_crash_on_failure(u64 size) {
	void* result = malloc(size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to malloc(%zx)", (size_t) size);
	DebugBreak();
	exit(-1);
}
void* realloc_crash_on_failure(void* data, u64 size) {
	void* result = realloc(data, size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to realloc(%p, %zx)", data, (size_t) size);
	DebugBreak();
	exit(-1);
}

struct CRTAllocator {
	REFLECT(CRTAllocator) {}
};

void* c_allocator_proc(AllocatorProcParams params) {
	switch (params.verb) {
		case ALLOCATOR_VERB_ALLOC:
			return malloc_crash_on_failure(params.new_size);
		case ALLOCATOR_VERB_REALLOC:
			return realloc_crash_on_failure(params.old_data, params.new_size);
		case ALLOCATOR_VERB_FREE:
			free(params.old_data);
			break;
		case ALLOCATOR_VERB_GET_TYPE:
			return reflect_type_of<CRTAllocator>();
		default:
			assert(false);
			return NULL;
	}
	return NULL;
}

constexpr Allocator crt_allocator = {
	.proc           = &c_allocator_proc,
	.allocator_data = NULL,
};

// Aliasing like this allows us to replace c_allocator
//   with other allocator in runtime.
Allocator c_allocator = crt_allocator;

constexpr Allocator null_allocator = { 
	.proc           = NULL,
	.allocator_data = NULL,
};

void* Malloc(Allocator allocator, u64 size, CodeLocation loc = caller_loc()) {
	AllocatorProcParams p = {
		.verb = ALLOCATOR_VERB_ALLOC,
		.new_size = size,
		.allocator_data = allocator.allocator_data,
		.loc = loc
	};
	return allocator.proc(p).data;
}

void* Malloc(u64 size, CodeLocation loc = caller_loc()) {
	return Malloc(c_allocator, size, loc);
}

void Free(Allocator allocator, void* data, CodeLocation loc = caller_loc()) {
	if (data == NULL) {
		return;
	}
	AllocatorProcParams p = {
		.verb = ALLOCATOR_VERB_FREE,
		.old_data = data,
		.allocator_data = allocator.allocator_data,
		.loc = loc
	};
	allocator.proc(p);
}

void Free(void* data, CodeLocation loc = caller_loc()) {
	Free(c_allocator, data, loc);
}

void* Realloc(Allocator allocator, void* data, u64 old_size, u64 new_size, CodeLocation loc = caller_loc()) {
	AllocatorProcParams p = {
		.verb = ALLOCATOR_VERB_REALLOC,
		.old_data = data,
		.old_size = old_size,
		.new_size = new_size,
		.allocator_data = allocator.allocator_data,
		.loc = loc
	};
	return allocator.proc(p).data;
}

void* Realloc(void* data, u64 old_size, u64 new_size, CodeLocation loc = caller_loc()) {
	return Realloc(c_allocator, data, old_size, new_size, loc);
}

template <typename T>
T* Alloc(Allocator allocator, u64 count, CodeLocation loc = caller_loc()) {
	return (T*) Malloc(allocator, sizeof(T) * count, loc);
}

template <typename T>
T* Alloc(u64 count, CodeLocation loc = caller_loc()) {
	return Alloc<T>(c_allocator, count, loc);
}

template <typename T>
T* copy_value(Allocator allocator, T thing, CodeLocation loc = caller_loc()) {
	T* mem = Alloc<T>(1, loc);
	memcpy(mem, &thing, sizeof(T));
	return mem;
}

template <typename T>
T* make(Allocator allocator = c_allocator, CodeLocation loc = caller_loc()) {
	T* mem = Alloc<T>(allocator,1, loc);
	return new(mem) T();
}

template <typename T>
T* make(s64 count, Allocator allocator = c_allocator, CodeLocation loc = caller_loc()) {
	T* mem = (T*) Alloc<T>(allocator, count, loc); 
	for (auto i: range(count)) {
		new(mem + i) T();
	}
	return mem;
}
