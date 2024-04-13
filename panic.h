#pragma once

#include "format.h"
#include "sync/spinlock.h"
#include "code_location.h"

// One of reason we could call panic() is fail of 
//   heap allocator, so we have to have some way to allocate memory for error message.
constexpr u64 panic_message_memory_size = 16 * 1024; 
inline u8     panic_message_memory[panic_message_memory_size];
inline u64    panic_message_memory_offset = 0;

inline void* panic_allocator_proc(AllocatorVerb verb, void* old_data, u64 old_size, u64 size, void* allocator_data, CodeLocation code_location) {
	switch (verb) {
		case ALLOCATOR_VERB_ALLOC: {
			if (size + panic_message_memory_offset > panic_message_memory_size) {
				abort(); // Just die at this point.
				return NULL;
			}
			auto result = panic_message_memory + panic_message_memory_offset;
			panic_message_memory_offset += size;
			return result;
		}
		break;
		case ALLOCATOR_VERB_FREE: return NULL;
		case ALLOCATOR_VERB_REALLOC: return NULL;
		case ALLOCATOR_VERB_GET_TYPE: return NULL;
		case ALLOCATOR_VERB_FREE_ALLOCATOR: return NULL;
	}
	return NULL;
}

constexpr Allocator panic_allocator = {
	.proc           = &panic_allocator_proc,
	.allocator_data = NULL,
#ifdef ALLOCATOR_NAMES
	.name = "panic_allocator",
#endif
};

Spinlock PANIC_LOCK;

inline void panic(const char* file_name, int line_number, auto... args) {
	PANIC_LOCK.lock();
	auto message = sprint_unicode(panic_allocator, args...);
	String utf8 = encode_utf8(panic_allocator, message);
	fwrite(utf8.data, utf8.length, 1, stderr);
	fwrite("\n", 1, 1, stderr);
	Debug_Break();
	exit(-1);
}

#define panic(...) panic(__FILE__, __LINE__, __VA_ARGS__)
