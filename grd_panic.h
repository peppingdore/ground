#pragma once

#include "grd_format.h"
#include "sync/grd_spinlock.h"
#include "grd_code_location.h"
#include "grd_callstack.h"

// One of reason we could call panic() is fail of 
//   heap allocator, so we have to have some way to allocate memory for error message.
constexpr u64 grd_panic_message_memory_size = 16 * 1024; 
inline u8     grd_panic_message_memory[grd_panic_message_memory_size];
inline u64    grd_panic_message_memory_offset = 0;

GrdAllocatorProcResult grd_panic_allocator_proc(void* allocator_data, GrdAllocatorProcParams p) {
	switch (p.verb) {
		case GRD_ALLOCATOR_VERB_ALLOC: {
			if (p.new_size + grd_panic_message_memory_offset > grd_panic_message_memory_size) {
				abort(); // Just die at this point.
				return {};
			}
			auto result = grd_panic_message_memory + grd_panic_message_memory_offset;
			grd_panic_message_memory_offset += p.new_size;
			return { .data = result };
		}
		break;
		case GRD_ALLOCATOR_VERB_FREE: return {};
		case GRD_ALLOCATOR_VERB_REALLOC: {
			auto sub_p = p;
			sub_p.verb = GRD_ALLOCATOR_VERB_ALLOC;
			auto new_res = grd_panic_allocator_proc(allocator_data, sub_p);
			memcpy(new_res.data, p.old_data, grd_min_u64(p.old_size, p.new_size));
			sub_p = p;
			sub_p.verb = GRD_ALLOCATOR_VERB_FREE;
			grd_panic_allocator_proc(allocator_data, sub_p);
			return new_res;
		}
		break;
		case GRD_ALLOCATOR_VERB_GET_TYPE: return {};
		case GRD_ALLOCATOR_VERB_FREE_ALLOCATOR: return {};
		default:
			return {};
	}
	return {};
}

constexpr GrdAllocator grd_panic_allocator = {
	.proc = &grd_panic_allocator_proc,
	.data = NULL,
};

GrdSpinlock GRD_PANIC_LOCK;

void grd_panic_write_string(const char* str) {
	fwrite(str, strlen(str), 1, stderr);
}

inline void grd_panic(const char* file_name, int line_number, auto... args) {
	grd_lock(&GRD_PANIC_LOCK);
	auto message = grd_sprint_unicode(grd_panic_allocator, args...);
	GrdString utf8 = grd_encode_utf8(grd_panic_allocator, message);
	grd_panic_write_string("\n");
	grd_panic_write_string("Panic: ");
	fwrite(utf8.data, grd_len(utf8), 1, stderr);
	grd_panic_write_string("\n");
	grd_panic_write_string("  at:");
	// fwrite(file_name, strlen(file_name), 1, stderr);
	// fwrite(":", 1, 1, stderr);
	// char buf[32];
	// snprintf(buf, sizeof(buf), "%d", line_number);
	// fwrite(buf, strlen(buf), 1, stderr);
	grd_panic_write_string("\n");
	auto cs = grd_get_callstack();
	grd_print_callstack(cs, 1);
	GrdDebugBreak();
	exit(-1);
}

#define grd_panic(...) grd_panic(__FILE__, __LINE__, __VA_ARGS__)
