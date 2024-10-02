#pragma once

#include "format.h"
#include "sync/spinlock.h"
#include "code_location.h"
#include "callstack.h"

// One of reason we could call panic() is fail of 
//   heap allocator, so we have to have some way to allocate memory for error message.
constexpr u64 panic_message_memory_size = 16 * 1024; 
inline u8     panic_message_memory[panic_message_memory_size];
inline u64    panic_message_memory_offset = 0;

AllocatorProcResult panic_allocator_proc(void* allocator_data, AllocatorProcParams p) {
	switch (p.verb) {
		case ALLOCATOR_VERB_ALLOC: {
			if (p.new_size + panic_message_memory_offset > panic_message_memory_size) {
				abort(); // Just die at this point.
				return {};
			}
			auto result = panic_message_memory + panic_message_memory_offset;
			panic_message_memory_offset += p.new_size;
			return { .data = result };
		}
		break;
		case ALLOCATOR_VERB_FREE: return {};
		case ALLOCATOR_VERB_REALLOC: {
			auto sub_p = p;
			sub_p.verb = ALLOCATOR_VERB_ALLOC;
			auto new_res = panic_allocator_proc(allocator_data, sub_p);
			memcpy(new_res.data, p.old_data, min_u64(p.old_size, p.new_size));
			sub_p = p;
			sub_p.verb = ALLOCATOR_VERB_FREE;
			panic_allocator_proc(allocator_data, sub_p);
			return new_res;
		}
		break;
		case ALLOCATOR_VERB_GET_TYPE: return {};
		case ALLOCATOR_VERB_FREE_ALLOCATOR: return {};
		default:
			return {};
	}
	return {};
}

constexpr Allocator panic_allocator = {
	.proc = &panic_allocator_proc,
	.data = NULL,
};

Spinlock PANIC_LOCK;

void panic_write_string(const char* str) {
	fwrite(str, strlen(str), 1, stderr);
}

inline void panic(const char* file_name, int line_number, auto... args) {
	PANIC_LOCK.lock();
	auto message = sprint_unicode(panic_allocator, args...);
	String utf8 = encode_utf8(panic_allocator, message);
	panic_write_string("\n");
	panic_write_string("Panic: ");
	fwrite(utf8.data, len(utf8), 1, stderr);
	panic_write_string("\n");
	panic_write_string("  at:");
	// fwrite(file_name, strlen(file_name), 1, stderr);
	// fwrite(":", 1, 1, stderr);
	// char buf[32];
	// snprintf(buf, sizeof(buf), "%d", line_number);
	// fwrite(buf, strlen(buf), 1, stderr);
	panic_write_string("\n");
	auto cs = get_callstack();
	print_callstack(cs, 1);
	DebugBreak();
	exit(-1);
}

#define panic(...) panic(__FILE__, __LINE__, __VA_ARGS__)
