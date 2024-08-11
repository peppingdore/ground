#pragma once

#include <stacktrace>
#include "format.h"

struct CallStackEntry {
	CodeLocation loc;
	const char*  desc = NULL;
};

struct CallStack {
	Allocator       allocator;
	CallStackEntry* entries = NULL;
	s64             count = 0;
};

void free_callstack(CallStack st) {
	for (auto i: range(st.count)) {
		Free(st.allocator, (void*) st.entries[i].loc.file);
		Free(st.allocator, (void*) st.entries[i].desc);
	}
}

const char* callstack_copy_std_string(Allocator allocator, std::string&& str) {
	auto str_mem = Alloc<char>(allocator, str.size() + 1);
	for (auto i: range(str.size())) {
		str_mem[i] = str[i];
	}
	str_mem[str.size()] = '\0';
	return str_mem;
}

CallStack get_callstack(Allocator allocator = c_allocator) {
	auto cpp_st = std::stacktrace::current();
	CallStack st;
	st.allocator = allocator;
	// Skipping first entry to skip get_callstack()'s frame.
	st.count = cpp_st.size() - 1;
	st.entries = Alloc<CallStackEntry>(allocator, st.count);
	for (auto i: range_from_to(1, st.count + 1)) {
		auto& x = cpp_st[i];
		auto f_str = callstack_copy_std_string(allocator, x.source_file());
		st.entries[i - 1].loc = make_code_location(x.source_line(), f_str);
		st.entries[i - 1].desc = callstack_copy_std_string(allocator, x.description());
	}
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
