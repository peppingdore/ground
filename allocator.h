#pragma once

#include "base.h"
#include "range.h"
#include "code_location.h"
#include <cstdlib>
#include <new>

enum Allocator_Verb: s32 {
	ALLOCATOR_VERB_ALLOC   = 0,
	ALLOCATOR_VERB_REALLOC = 1,
	ALLOCATOR_VERB_FREE    = 2,
};

using Allocator_Proc = void* (Allocator_Verb, void* old_data, u64 old_size, u64 new_size, void* allocator_data, Code_Location);

constexpr u32 ALLOCATOR_HAS_NO_FREE_AND_REALLOC = 0;
constexpr u32 ALLOCATOR_HAS_REALLOC             = 1 << 0;
constexpr u32 ALLOCATOR_HAS_FREE                = 1 << 1;
constexpr u32 ALLOCATOR_IS_ARENA                = 1 << 3;

struct Allocator {
	Allocator_Proc* proc;
	void*           allocator_data;
	u32             flags;

#ifdef ALLOCATOR_NAMES
	const char* name;
#endif

	void* alloc(u64 size, Code_Location loc = caller_loc()) {
		return proc(ALLOCATOR_VERB_ALLOC, NULL, 0, size, allocator_data, loc);
	}

	void free(void* data, Code_Location loc = caller_loc()) {
		if (data == NULL) {
			return;
		}
		proc(ALLOCATOR_VERB_FREE, data, 0, 0, allocator_data, loc);
	}

	void* realloc(void* data, u64 old_size, u64 new_size, Code_Location loc = caller_loc()) {
		if (flags & ALLOCATOR_HAS_REALLOC) {
			return proc(ALLOCATOR_VERB_REALLOC, data, old_size, new_size, allocator_data, loc);
		} else {
			void* new_data = alloc(new_size, loc);			
			memcpy(new_data, data, min(old_size, new_size));
			free(data, loc);
			return new_data;
		}
	}

	template <typename T>
	T* alloc(u64 count, Code_Location loc = caller_loc()) {
		return (T*) alloc(sizeof(T) * count, loc);
	}

	template <typename T>
	T* realloc(T* old_data, u64 old_count, u64 new_count, Code_Location loc = caller_loc()) {
		return (T*) realloc((void*) old_data, old_count * sizeof(T), new_count * sizeof(T), loc);
	}

	bool operator==(Allocator rhs) {
		return proc == rhs.proc && allocator_data == rhs.allocator_data;
	}
};


inline void* malloc_crash_on_failure(u64 size) {
	void* result = malloc(size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to malloc(%zx)", (size_t) size);
	exit(-1);
}
inline void* realloc_crash_on_failure(void* data, u64 size) {
	void* result = realloc(data, size);
	if (result) {
		return result;
	}
	fprintf(stderr, "Failed to realloc(%p, %zx)", data, (size_t) size);
	exit(-1);
}

inline void* c_allocator_proc(Allocator_Verb verb, void* old_data, u64 old_size, u64 size, void* allocator_data, Code_Location loc) {

	switch (verb) {
		case ALLOCATOR_VERB_ALLOC:
			return malloc_crash_on_failure(size);
		case ALLOCATOR_VERB_REALLOC:
			return realloc_crash_on_failure(old_data, size);
		case ALLOCATOR_VERB_FREE:
			free(old_data);
			break;
		default:
			assert(false);
			return NULL;
	}
	return NULL;
}

constexpr Allocator __crt_allocator = {
	.proc           = &c_allocator_proc,
	.allocator_data = NULL,
	.flags          = ALLOCATOR_HAS_FREE | ALLOCATOR_HAS_REALLOC,

#ifdef ALLOCATOR_NAMES
	.name = "c_allocator",
#endif
};

// Aliasing like this allows us to replace c_allocator
//   with other allocator at the start of the program.
Allocator c_allocator = __crt_allocator;

constexpr Allocator null_allocator = { 
	.proc           = NULL,
	.allocator_data = NULL,
	.flags          = 0,

#ifdef ALLOCATOR_NAMES
	.name = "null_allocator",
#endif
};

template <typename T>
inline T* copy(Allocator allocator, T thing, Code_Location loc = caller_loc())
{
	T* mem = allocator.alloc<T>(1, loc);
	memcpy(mem, &thing, sizeof(T));
	return mem;
}

template <typename T>
inline T* make(Allocator allocator = c_allocator, Code_Location loc = caller_loc())
{
	T* mem = allocator.alloc<T>(1, loc);
	return new(mem) T();
}

template <typename T>
inline T* make(s64 count, Allocator allocator = c_allocator, Code_Location loc = caller_loc()) {
	T* mem = (T*) allocator.alloc(sizeof(T) * count, loc);
	for (auto i: range(count)) {
		new(mem + i) T();
	}
	return mem;
}
