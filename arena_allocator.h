#pragma once

#include "allocator.h"
#include "panic.h"

constexpr u64 DEFAULT_ARENA_SIZE = 16 * 1024;

struct Arena {
	u64       allocated = 0;
	Arena*    next = NULL;
};

struct LinkedArenas {
	Allocator parent_allocator;
	u64       arena_size = 0;
	Arena     first;
};

void* get_arena_mem_block(LinkedArenas* arenas, Arena* arena) {
	if (&arenas->first == arena) {
		return arenas;
	}
	return arena;
}

void free_linked_arenas(LinkedArenas* arenas) {
	auto arena = &arenas->first;
	auto parent_allocator = arenas->parent_allocator;
	while (arena) {
		auto block = get_arena_mem_block(arenas, arena);
		arena = arena->next;
		Free(parent_allocator, block);
	}
}

Arena* make_arena(Allocator parent_allocator, u64 size) {
	Arena* arena = (Arena*) Malloc(parent_allocator, sizeof(Arena) + size);
	*arena = Arena{};
	return arena;
}

AllocatorProcResult arena_allocator_proc(void* allocator_data, AllocatorProcParams p) {
	auto arenas = (LinkedArenas*) allocator_data;
	switch (p.verb) {
		case ALLOCATOR_VERB_ALLOC: {
			Arena* last = &arenas->first;
			Arena* found = NULL;
			while (true) {
				if (last->allocated + p.new_size <= arenas->arena_size) { 
					found = last;
					break;
				}
				if (last->allocated != arenas->arena_size) {
					last->allocated = arenas->arena_size;
				}
				if (!last->next) {
					break;
				}
				last = last->next;
			}
			if (!found) {
				u64 new_arena_size = max_u64(arenas->arena_size, p.new_size);
				last = make_arena(arenas->parent_allocator, new_arena_size);
			}
			void* ptr = ptr_add(last, sizeof(Arena) + last->allocated);
			last->allocated += p.new_size;
			return { .data = ptr };
		}
		break;
		case ALLOCATOR_VERB_REALLOC: {
			auto res = arena_allocator_proc(allocator_data, { .verb = ALLOCATOR_VERB_ALLOC, .new_size = p.new_size, .loc = p.loc });
			memcpy(res.data, p.old_data, min_u64(p.old_size, p.new_size));
			arena_allocator_proc(allocator_data, { .verb = ALLOCATOR_VERB_FREE, .old_data = p.old_data, .loc = p.loc });
			return res;
		}
		case ALLOCATOR_VERB_FREE:
			return {};
		case ALLOCATOR_VERB_GET_TYPE:
			return { .allocator_type = reflect_type_of<LinkedArenas>() };
		case ALLOCATOR_VERB_FREE_ALLOCATOR:
			free_linked_arenas(arenas);
			break;
	}
	return {};
}

Allocator make_arena_allocator(Allocator parent_allocator, u64 arena_size = DEFAULT_ARENA_SIZE) {
	if (arena_size == 0) {
		panic("Arena size must be greater than 0");
	}

	auto arenas = (LinkedArenas*) Malloc(parent_allocator, sizeof(LinkedArenas) + arena_size);
	*arenas = LinkedArenas {
		.parent_allocator = parent_allocator,
		.arena_size = arena_size,
	};
	Allocator allocator = {
		.proc = arena_allocator_proc,
		.data = arenas,
	};
	return allocator;
}

Allocator make_arena_allocator(u64 arena_size = DEFAULT_ARENA_SIZE) {
	return make_arena_allocator(c_allocator, arena_size);
}

struct ArenaAllocatorSnapshot {
	s64 current_arena_index = 0;
	u64 current_arena_allocated = 0;
};

ArenaAllocatorSnapshot snapshot(LinkedArenas* allocator) {
	s64 current_arena_index = 0;
	u64 current_arena_allocated = 0;

	auto arena = &allocator->first;
	while (arena) {
		current_arena_allocated = arena->allocated;
		if (arena->allocated < allocator->arena_size) {
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

void restore(LinkedArenas* allocator, ArenaAllocatorSnapshot snapshot) {
	auto arena = &allocator->first;
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

ArenaAllocatorSnapshot snapshot(Allocator allocator) {
	auto type = get_allocator_type(allocator);
	if (type != reflect_type_of<LinkedArenas>()) {
		return {};
	}
	return snapshot((LinkedArenas*) allocator.data);
}

void restore(Allocator allocator, ArenaAllocatorSnapshot snapshot) {
	auto type = get_allocator_type(allocator);
	if (type != reflect_type_of<LinkedArenas>()) {
		return;
	}
	restore((LinkedArenas*) allocator.data, snapshot);
}
