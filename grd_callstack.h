#pragma once

#if __cpp_lib_stacktrace
	#include <stacktrace>
#else
	#include "grd_build.h"
	#if GRD_OS_LINUX
		GRD_BUILD_RUN("params.add_lib('dwarf')")
		GRD_BUILD_RUN("params.add_lib('elf')")
		#define BACKWARD_HAS_DWARF 1
	#endif
	#include "third_party/backward_cpp.h"
#endif

#include <string>
#include "grd_format.h"

struct GrdCallStackEntry {
	GrdCodeLoc   loc;
	const char*  desc = NULL;
};

struct GrdCallStack {
	GrdAllocator       allocator;
	GrdCallStackEntry* entries = NULL;
	s64                count = 0;
};

void grd_free_callstack(GrdCallStack st) {
	for (auto i: grd_range(st.count)) {
		GrdFree(st.allocator, (void*) st.entries[i].loc.file);
		GrdFree(st.allocator, (void*) st.entries[i].desc);
	}
}

const char* grd_callstack_copy_std_string(GrdAllocator allocator, std::string&& str) {
	auto str_mem = GrdAlloc<char>(allocator, str.size() + 1);
	for (auto i: grd_range(str.size())) {
		str_mem[i] = str[i];
	}
	str_mem[str.size()] = '\0';
	return str_mem;
}

GrdCallStack grd_get_callstack(GrdAllocator allocator = c_allocator) {
	GrdCallStack st;
	st.allocator = allocator;
	#if __cpp_lib_stacktrace
		auto cpp_st = std::stacktrace::current();
		// Skipping first entry to skip grd_get_callstack()'s frame.
		st.count = cpp_st.size() - 1;
		st.entries = GrdAlloc<GrdCallStackEntry>(allocator, st.count);
		for (auto i: grd_range_from_to(1, st.count + 1)) {
			auto& x = cpp_st[i];
			auto f_str = grd_callstack_copy_std_string(allocator, x.source_file());
			st.entries[i - 1].loc = grd_make_code_loc(x.source_line(), f_str);
			st.entries[i - 1].desc = grd_callstack_copy_std_string(allocator, x.description());
		}
	#endif
	return st;
}

void grd_print_callstack_verbose(GrdCallStack st, s64 start_entry_idx = 0) {
	for (auto i: grd_range_from_to(start_entry_idx, st.count)) {
		auto entry = st.entries[i];
		printf("%s: %d  %s\n", entry.loc.file, entry.loc.line, entry.desc);
	}
}

void grd_print_callstack(GrdCallStack st, s64 start_entry_idx = 0) {
	s64 end = st.count;
	for (auto i: grd_reverse(grd_range_from_to(start_entry_idx, st.count))) {
		if (st.entries[i].loc.file != NULL && strlen(st.entries[i].loc.file) > 0) {
			break;
		}
		end = i;
	}
	for (auto i: grd_range_from_to(start_entry_idx, end)) {
		auto entry = st.entries[i];
		printf("%s: %d\n", entry.loc.file, entry.loc.line);
	}
}
