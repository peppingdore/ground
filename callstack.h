#pragma once

#if __cpp_lib_stacktrace
	#include <stacktrace>
#endif

#include <string>
#include "format.h"

struct CallStackEntry {
	GrdCodeLoc loc;
	const char*  desc = NULL;
};

struct CallStack {
	GrdAllocator       allocator;
	CallStackEntry* entries = NULL;
	s64             count = 0;
};

void free_callstack(CallStack st) {
	for (auto i: grd_range(st.count)) {
		GrdFree(st.allocator, (void*) st.entries[i].loc.file);
		GrdFree(st.allocator, (void*) st.entries[i].desc);
	}
}

const char* callstack_copy_std_string(GrdAllocator allocator, std::string&& str) {
	auto str_mem = Alloc<char>(allocator, str.size() + 1);
	for (auto i: grd_range(str.size())) {
		str_mem[i] = str[i];
	}
	str_mem[str.size()] = '\0';
	return str_mem;
}

CallStack get_callstack(GrdAllocator allocator = c_allocator) {
	CallStack st;
	st.allocator = allocator;
	#if __cpp_lib_stacktrace
		auto cpp_st = std::stacktrace::current();
		// Skipping first entry to skip get_callstack()'s frame.
		st.count = cpp_st.size() - 1;
		st.entries = Alloc<CallStackEntry>(allocator, st.count);
		for (auto i: range_from_to(1, st.count + 1)) {
			auto& x = cpp_st[i];
			auto f_str = callstack_copy_std_string(allocator, x.source_file());
			st.entries[i - 1].loc = grd_grd_make_code_loc(x.source_line(), f_str);
			st.entries[i - 1].desc = callstack_copy_std_string(allocator, x.description());
		}
	#endif
	return st;
}

void print_callstack_verbose(CallStack st, s64 start_entry_idx = 0) {
	for (auto i: range_from_to(start_entry_idx, st.count)) {
		auto entry = st.entries[i];
		printf("%s: %d  %s\n", entry.loc.file, entry.loc.line, entry.desc);
	}
}

void print_callstack(CallStack st, s64 start_entry_idx = 0) {
	s64 end = st.count;
	for (auto i: reverse(range_from_to(start_entry_idx, st.count))) {
		if (st.entries[i].loc.file != NULL && strlen(st.entries[i].loc.file) > 0) {
			break;
		}
		end = i;
	}
	for (auto i: range_from_to(start_entry_idx, end)) {
		auto entry = st.entries[i];
		printf("%s: %d\n", entry.loc.file, entry.loc.line);
	}
}
