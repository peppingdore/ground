#pragma once

#include "base.h"
#include "range.h"
#include "code_location.h"
#include "math/basic_functions.h"

#include <cstdlib>
#include <new>
#include <stdio.h>

enum AllocatorVerb: s32 {
	ALLOCATOR_VERB_ALLOC     = 0,
	ALLOCATOR_VERB_REALLOC   = 1,
	ALLOCATOR_VERB_FREE      = 2,
	ALLOCATOR_VERB_GET_NAME  = 3,
	ALLOCATOR_VERB_GET_FLAGS = 4,
};

using AllocatorProc = void* (AllocatorVerb, void* old_data, u64 old_size, u64 new_size, void* allocator_data, CodeLocation);

enum AllocatorFlags: u32 {
	ALLOCATOR_DEFAULT_FLAGS   = 0,
	ALLOCATOR_FLAG_NO_REALLOC = 1 << 0,
	ALLOCATOR_FLAG_NO_FREE    = 1 << 1,
	ALLOCATOR_FLAG_IS_ARENA   = 1 << 3,
};

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

u32 get_allocator_flags(Allocator allocator) {
	return (AllocatorFlags) (u64) allocator.proc(ALLOCATOR_VERB_GET_FLAGS, NULL, 0, 0, allocator.allocator_data, current_loc());
}

const char* get_allocator_name(Allocator allocator) {
	return (const char*) allocator.proc(ALLOCATOR_VERB_GET_NAME, NULL, 0, 0, allocator.allocator_data, current_loc());
}

void* alloc(Allocator allocator, u64 size, CodeLocation loc = caller_loc()) {
	return allocator.proc(ALLOCATOR_VERB_ALLOC, NULL, 0, size, allocator.allocator_data, loc);
}

void free(Allocator allocator, void* data, CodeLocation loc = caller_loc()) {
	if (data == NULL) {
		return;
	}
	allocator.proc(ALLOCATOR_VERB_FREE, data, 0, 0, allocator.allocator_data, loc);
}

void* realloc(Allocator allocator, void* data, u64 old_size, u64 new_size, CodeLocation loc = caller_loc()) {
	auto flags = get_allocator_flags(allocator);
	if (flags & ALLOCATOR_FLAG_NO_REALLOC) {
		void* new_data = alloc(allocator, new_size, loc);			
		memcpy(new_data, data, min(old_size, new_size));
		free(allocator, data, loc);
		return new_data;
	}
	return allocator.proc(ALLOCATOR_VERB_REALLOC, data, old_size, new_size, allocator.allocator_data, loc);
}

template <typename T>
T* alloc(Allocator allocator, u64 count, CodeLocation loc = caller_loc()) {
	return (T*) alloc(allocator, sizeof(T) * count, loc);
}

template <typename T>
T* realloc(Allocator allocator, T* old_data, u64 old_count, u64 new_count, CodeLocation loc = caller_loc()) {
	return (T*) realloc(allocator, (void*) old_data, old_count * sizeof(T), new_count * sizeof(T), loc);
}

void* malloc_crash_on_failure(u64 size) {
	void* result = malloc(size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to malloc(%zx)", (size_t) size);
	exit(-1);
}
void* realloc_crash_on_failure(void* data, u64 size) {
	void* result = realloc(data, size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to realloc(%p, %zx)", data, (size_t) size);
	exit(-1);
}

void* c_allocator_proc(AllocatorVerb verb, void* old_data, u64 old_size, u64 size, void* allocator_data, CodeLocation loc) {
	switch (verb) {
		case ALLOCATOR_VERB_ALLOC:
			return malloc_crash_on_failure(size);
		case ALLOCATOR_VERB_REALLOC:
			return realloc_crash_on_failure(old_data, size);
		case ALLOCATOR_VERB_FREE:
			free(old_data);
			break;
		case ALLOCATOR_VERB_GET_FLAGS:
			return (void*) (u64) ALLOCATOR_DEFAULT_FLAGS;
		case ALLOCATOR_VERB_GET_NAME:
			return (void*) "crt_allocator";
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

template <typename T>
T* copy(Allocator allocator, T thing, CodeLocation loc = caller_loc()) {
	T* mem = allocator.alloc<T>(1, loc);
	memcpy(mem, &thing, sizeof(T));
	return mem;
}

template <typename T>
T* make(Allocator allocator = c_allocator, CodeLocation loc = caller_loc()) {
	T* mem = allocator.alloc<T>(1, loc);
	return new(mem) T();
}

template <typename T>
T* make(s64 count, Allocator allocator = c_allocator, CodeLocation loc = caller_loc()) {
	T* mem = (T*) allocator.alloc(sizeof(T) * count, loc);
	for (auto i: range(count)) {
		new(mem + i) T();
	}
	return mem;
}
