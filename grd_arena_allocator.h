#pragma once

#include "grd_allocator.h"
#include "grd_panic.h"

constexpr u64 GRD_DEFAULT_ARENA_SIZE = 16 * 1024;

struct GrdArena {
	u64       allocated = 0;
	GrdArena* next = NULL;
};

struct GrdLinkedArenas {
	GrdAllocator parent_allocator;
	u64          arena_size = 0;
	GrdArena     first;
};

GRD_DEDUP void* grd_get_arena_mem_block(GrdLinkedArenas* arenas, GrdArena* arena) {
	if (&arenas->first == arena) {
		return arenas;
	}
	return arena;
}

GRD_DEDUP void grd_free_linked_arenas(GrdLinkedArenas* arenas) {
	auto arena = &arenas->first;
	auto parent_allocator = arenas->parent_allocator;
	while (arena) {
		auto block = grd_get_arena_mem_block(arenas, arena);
		arena = arena->next;
		GrdFree(parent_allocator, block);
	}
}

GRD_DEDUP GrdArena* grd_make_arena(GrdAllocator parent_allocator, u64 size) {
	GrdArena* arena = (GrdArena*) GrdMalloc(parent_allocator, sizeof(GrdArena) + size);
	*arena = GrdArena{};
	return arena;
}

GRD_DEDUP GrdAllocatorProcResult grd_arena_allocator_proc(void* allocator_data, GrdAllocatorProcParams p) {
	auto arenas = (GrdLinkedArenas*) allocator_data;
	switch (p.verb) {
		case GRD_ALLOCATOR_VERB_ALLOC: {
			GrdArena* last = &arenas->first;
			GrdArena* found = NULL;
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
				u64 new_arena_size = grd_max_u64(arenas->arena_size, p.new_size);
				last = grd_make_arena(arenas->parent_allocator, new_arena_size);
			}
			void* ptr = grd_ptr_add(last, sizeof(GrdArena) + last->allocated);
			last->allocated += p.new_size;
			return { .data = ptr };
		}
		break;
		case GRD_ALLOCATOR_VERB_REALLOC: {
			auto res = grd_arena_allocator_proc(allocator_data, { .verb = GRD_ALLOCATOR_VERB_ALLOC, .new_size = p.new_size, .loc = p.loc });
			memcpy(res.data, p.old_data, grd_min_u64(p.old_size, p.new_size));
			grd_arena_allocator_proc(allocator_data, { .verb = GRD_ALLOCATOR_VERB_FREE, .old_data = p.old_data, .loc = p.loc });
			return res;
		}
		case GRD_ALLOCATOR_VERB_FREE:
			return {};
		case GRD_ALLOCATOR_VERB_GET_TYPE:
			return { .allocator_type = grd_reflect_type_of<GrdLinkedArenas>() };
		case GRD_ALLOCATOR_VERB_FREE_ALLOCATOR:
			grd_free_linked_arenas(arenas);
			break;
	}
	return {};
}

GRD_DEDUP GrdAllocator grd_make_arena_allocator(GrdAllocator parent_allocator, u64 arena_size = GRD_DEFAULT_ARENA_SIZE) {
	if (arena_size == 0) {
		grd_panic("GrdArena size must be greater than 0");
	}

	auto arenas = (GrdLinkedArenas*) GrdMalloc(parent_allocator, sizeof(GrdLinkedArenas) + arena_size);
	*arenas = GrdLinkedArenas {
		.parent_allocator = parent_allocator,
		.arena_size = arena_size,
	};
	GrdAllocator allocator = {
		.proc = grd_arena_allocator_proc,
		.data = arenas,
	};
	return allocator;
}

GRD_DEDUP GrdAllocator grd_make_arena_allocator(u64 arena_size = GRD_DEFAULT_ARENA_SIZE) {
	return grd_make_arena_allocator(c_allocator, arena_size);
}

struct ArenaAllocatorSnapshot {
	s64 current_arena_index = 0;
	u64 current_arena_allocated = 0;
};

GRD_DEDUP ArenaAllocatorSnapshot grd_snapshot(GrdLinkedArenas* allocator) {
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

GRD_DEDUP void grd_restore(GrdLinkedArenas* allocator, ArenaAllocatorSnapshot snapshot) {
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

GRD_DEDUP ArenaAllocatorSnapshot grd_snapshot(GrdAllocator allocator) {
	auto type = grd_get_allocator_type(allocator);
	if (type != grd_reflect_type_of<GrdLinkedArenas>()) {
		return {};
	}
	return grd_snapshot((GrdLinkedArenas*) allocator.data);
}

GRD_DEDUP void grd_restore(GrdAllocator allocator, ArenaAllocatorSnapshot snapshot) {
	auto type = grd_get_allocator_type(allocator);
	if (type != grd_reflect_type_of<GrdLinkedArenas>()) {
		return;
	}
	grd_restore((GrdLinkedArenas*) allocator.data, snapshot);
}
