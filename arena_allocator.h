#pragma once

#include "allocator.h"
#include "panic.h"

struct Arena {
	void*     mem;
	u64       allocated = 0;
	Arena*    next = NULL;
};

Arena* make_arena(Allocator allocator, u64 capacity) {
	auto arena = make<Arena>(allocator);
	arena->mem = alloc(allocator, capacity);
	return arena;
}

struct ArenaAllocator {
	Allocator parent_allocator;
	Arena*    first = NULL;
	u64       arena_size = 0;

	void free() {
		auto arena = first;
		while (arena) {
			::free(parent_allocator, arena->mem);
			arena = arena->next;
		}
		first = NULL;
	}
};

void* arena_allocator_proc(AllocatorVerb verb, void* old_data, u64 old_size, u64 size, void* allocator_data, CodeLocation loc) {
	switch (verb) {
		case ALLOCATOR_VERB_ALLOC: {
			auto arena_allocator = (ArenaAllocator*) allocator_data;
			Arena* last = arena_allocator->first;
			Arena* found = NULL;
			while (true) {
				if (last->allocated + size <= arena_allocator->arena_size) { 
					found = last;
					break;
				}
				if (last->allocated != arena_allocator->arena_size) {
					last->allocated = arena_allocator->arena_size;
				}
				if (!last->next) {
					break;
				}
				last = last->next;
			}
			if (!found) {
				u64 size = max(arena_allocator->arena_size, size);
				last = make_arena(arena_allocator->parent_allocator, size);
			}
			void* ptr = ptr_add(last->mem, last->allocated);
			last->allocated += size;
			return ptr;
		}
		break;
		case ALLOCATOR_VERB_GET_FLAGS:
			return (void*) (u64) (ALLOCATOR_FLAG_NO_FREE | ALLOCATOR_FLAG_NO_REALLOC | ALLOCATOR_FLAG_IS_ARENA);
		case ALLOCATOR_VERB_GET_NAME:
			return (void*) "arena_allocator";
	}
	return NULL;
}

Allocator make_arena_allocator(Allocator parent_allocator, u64 arena_size) {
	if (arena_size == 0) {
		panic("Arena size must be greater than 0");
	}

	auto arena_allocator = make<ArenaAllocator>(parent_allocator);
	arena_allocator->parent_allocator = parent_allocator;
	arena_allocator->arena_size = arena_size;
	arena_allocator->first = make_arena(parent_allocator, arena_size);

	Allocator allocator = {
		.proc = arena_allocator_proc,
		.allocator_data = arena_allocator,
	};
	return allocator;
}

struct ArenaAllocatorSnapshot {
	s64 current_arena_index = 0;
	u64 current_arena_allocated = 0;
};

ArenaAllocatorSnapshot snapshot(ArenaAllocator allocator) {
	s64 current_arena_index = 0;
	u64 current_arena_allocated = 0;

	auto arena = allocator.first;
	while (arena) {
		current_arena_allocated == arena->allocated;
		if (arena->allocated < allocator.arena_size) {
			break;
		}
		current_arena_index += 1;
		arena = arena->next;
	}

	return {
		.current_arena_index = current_arena_index,
		.current_arena_allocated = current_arena_allocated,
	};
}

void restore(ArenaAllocator* allocator, ArenaAllocatorSnapshot snapshot) {
	auto arena = allocator->first;
	s32 i = 0;
	while (arena) {
		if (i < snapshot.current_arena_index) {
			arena->allocated = allocator->arena_size;
		} if (i == snapshot.current_arena_index) {
			arena->allocated = snapshot.current_arena_allocated;
		} else {
			arena->allocated = 0;
		}
		arena = arena->next;
		i += 1;
	}
}
