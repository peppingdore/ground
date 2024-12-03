#pragma once

#define GRD_USE_STD_STACKTRACE (__cpp_lib_stacktrace && 0)

#if GRD_USE_STD_STACKTRACE
	#include <stacktrace>
#else
	#include "grd_build.h"
	#if GRD_OS_LINUX
		GRD_BUILD_RUN("ctx.params.add_lib('dwarf')")
		GRD_BUILD_RUN("ctx.params.add_lib('elf')")
		#define BACKWARD_HAS_DWARF 1
	#endif
	#if GRD_OS_DARWIN
		#define BACKWARD_HAS_DWARF 1
		#define BACKWARD_HAS_LIBUNWIND 1
	#endif
	#include "third_party/backward_cpp/backward_cpp.h"
	#define GRD_USE_BACKWARD_CPP_STACKTRACE 1
#endif

#include <string>
#include "grd_defer.h"
#include "grd_allocator.h"
#include "grd_code_location.h"
#include "sync/grd_spinlock.h"
#include <stdarg.h>

struct GrdStackTraceEntry {
	void*        addr = NULL;
	const char*  obj = NULL;
	const char*  obj_func = NULL;
	GrdCodeLoc   src_loc;
	const char*  src_func = NULL;
};

struct GrdStackTrace {
	GrdAllocator        allocator;
	GrdStackTraceEntry* entries = NULL;
	s64                 count = 0;

	void free() {
		for (auto i: grd_range(count)) {
			GrdFree(allocator, (void*) entries[i].obj);
			GrdFree(allocator, (void*) entries[i].obj_func);
			GrdFree(allocator, (void*) entries[i].src_loc.file);
			GrdFree(allocator, (void*) entries[i].src_func);
		}
	}
};

const char* grd_stack_trace_copy_std_str(GrdAllocator allocator, std::string& str) {
	auto str_mem = GrdAlloc<char>(allocator, str.size() + 1);
	for (auto i: grd_range(str.size())) {
		str_mem[i] = str[i];
	}
	str_mem[str.size()] = '\0';
	return str_mem;
}

GrdStackTrace grd_get_stack_trace(GrdAllocator allocator = c_allocator) {
	GrdStackTrace st;
	st.allocator = allocator;
	#if GRD_USE_STD_STACKTRACE
		auto cpp_st = std::stacktrace::current();
		// Skipping first entry to skip grd_get_callstack()'s frame.
		st.count = cpp_st.size() - 1;
		st.entries = GrdAlloc<GrdStackTraceEntry>(allocator, st.count);
		for (auto i: grd_range_from_to(1, st.count + 1)) {
			auto& x = cpp_st[i];
			auto f_str = grd_stack_trace_copy_std_str(allocator, x.source_file());
			st.entries[i - 1].loc = grd_make_code_loc(x.source_line(), f_str);
			st.entries[i - 1].desc = grd_stack_trace_copy_std_str(allocator, x.description());
		}
	#elif GRD_USE_BACKWARD_CPP_STACKTRACE
	
	#if GRD_OS_WINDOWS
		static GrdSpinlock symbol_lock;
		grd_lock(&symbol_lock);
		SymInitialize(GetCurrentProcess(), NULL, TRUE);
		grd_defer {
			SymCleanup(GetCurrentProcess());
			grd_unlock(&symbol_lock);
		};
	#endif

		backward::StackTrace b_st;
		b_st.load_here(32);
		#if GRD_OS_WINDOWS
			b_st.skip_n_firsts(2);
		#elif GRD_OS_DARWIN
			b_st.skip_n_firsts(1);
		#elif GRD_OS_LINUX
			b_st.skip_n_firsts(3);
		#endif
		backward::TraceResolver tr;
		tr.load_stacktrace(b_st);
		st.count = b_st.size();
		st.entries = GrdAlloc<GrdStackTraceEntry>(allocator, st.count);
		for (auto i: grd_range(st.count)) {
			backward::ResolvedTrace trace = tr.resolve(b_st[i]);
			auto entry = &st.entries[i];
			entry->addr = trace.addr;
			entry->obj = grd_stack_trace_copy_std_str(allocator, trace.object_filename);
			entry->obj_func = grd_stack_trace_copy_std_str(allocator, trace.object_function);
			entry->src_func = grd_stack_trace_copy_std_str(allocator, trace.source.function);
			entry->src_loc = grd_make_code_loc(trace.source.line, grd_stack_trace_copy_std_str(allocator, trace.source.filename));
			// printf("addr: %p\n", entry->addr);
			// printf("obj: %s\n", entry->obj);
			// printf("obj_func: %s\n", entry->obj_func);
			// printf("src_func: %s\n", entry->src_func);
			// printf("src_loc: %d %s\n", entry->src_loc.line, entry->src_loc.file);
		}
	#endif
	return st;
}

void grd_stack_trace_sprintf(char** buf, s64* buf_len, const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);
	s64 written = vsnprintf(*buf, *buf_len, fmt, args);
	*buf += written;
	*buf_len -= written;
	if (*buf_len < 0) {
		*buf_len = 0;
	}
}

bool grd_print_stack_trace_src(GrdStackTraceEntry* entry, char** buf, s64* buf_len) {
	if (!entry->src_loc.file || entry->src_loc.line < 1) {
		return false;
	}
	auto file = fopen(entry->src_loc.file, "r");
	if (!file) {
		return false;
	}
	grd_defer { fclose(file); };
	char line[256];
	s64 line_idx = 0;
	s64 cnt = 0;
	const u64 LINE_OVERFLOW = 3;
	while (true) {
		auto str = fgets(line, sizeof(line), file);
		if (!str) {
			break;
		}
		u64 line_len = strlen(line);
		line_idx += 1;
		bool missing_line_break = line_len > 0 && line[line_len - 1] != '\n';
		if (
			entry->src_loc.line >= line_idx - LINE_OVERFLOW &&
			entry->src_loc.line <= line_idx + LINE_OVERFLOW)
		{
			grd_stack_trace_sprintf(buf, buf_len, "    ");
			for (auto i: grd_range(line_len)) {
				if (line[i] == '\t') {
					grd_stack_trace_sprintf(buf, buf_len, "    ");
				} else {
					grd_stack_trace_sprintf(buf, buf_len, "%c", line[i]);
				}
			}
			if (missing_line_break) {
				grd_stack_trace_sprintf(buf, buf_len, "\n");
			}
			if (line_idx == entry->src_loc.line) {
				grd_stack_trace_sprintf(buf, buf_len, "    ");
				bool struct_non_whitespace = false;
				for (auto i: grd_range(line_len)) {
					if (line[i] != '\t' && line[i] != ' ') {
						struct_non_whitespace = true;
					}
					grd_stack_trace_sprintf(buf, buf_len, "%.*s", line[i] == '\t' ? 4 : 1, struct_non_whitespace ? "~~~~" : "    ");
				}
				grd_stack_trace_sprintf(buf, buf_len, "\n");
			}
		}
		if (missing_line_break) {
			char c;
			while ((c = fgetc(file)) != EOF) { 
				if (c == '\n') {
					break;
				}
			}
		}
	}
	grd_stack_trace_sprintf(buf, buf_len, "\n");
	return true;
}

enum GrdStackTracePrintFlags {
	GRD_STACK_TRACE_PRINT_FLAG_NONE = 0,
	GRD_STACK_TRACE_PRINT_SOURCE = 1 << 0,
};

void grd_print_stack_trace(GrdStackTrace st, char* buf, s64 buf_len, u32 flags = GRD_STACK_TRACE_PRINT_SOURCE) {
	for (auto i: grd_reverse(grd_range(st.count))) {
		auto entry = &st.entries[i];
		// grd_stack_trace_sprintf(&buf, &buf_len, "#%d ", (s32) i);
		if (entry->src_loc.file && strcmp(entry->src_loc.file, "") != 0) {
			if (entry->src_func && strcmp(entry->src_func, "") != 0) {
				grd_stack_trace_sprintf(&buf, &buf_len, "%s", entry->src_func);
			} else {
				grd_stack_trace_sprintf(&buf, &buf_len, "%p", entry->addr);
			}
			// grd_stack_trace_sprintf(&buf, &buf_len, " at %s:%d ", entry->src_loc.file, entry->src_loc.line);
			grd_stack_trace_sprintf(&buf, &buf_len, "\n");
			grd_stack_trace_sprintf(&buf, &buf_len, "  %s:%d ", entry->src_loc.file, entry->src_loc.line);
			grd_stack_trace_sprintf(&buf, &buf_len, "\n");
			if (flags & GRD_STACK_TRACE_PRINT_SOURCE) {
				grd_print_stack_trace_src(entry, &buf, &buf_len);
			}
		} else {
			if (entry->obj_func && strcmp(entry->obj_func, "") != 0) {
				grd_stack_trace_sprintf(&buf, &buf_len, "%s", entry->obj_func);
			} else {
				grd_stack_trace_sprintf(&buf, &buf_len, "%p", entry->addr);
			}
			// grd_stack_trace_sprintf(&buf, &buf_len, " at %s", entry->obj);
			grd_stack_trace_sprintf(&buf, &buf_len, "\n");
			grd_stack_trace_sprintf(&buf, &buf_len, "  %s", entry->obj);
			grd_stack_trace_sprintf(&buf, &buf_len, "\n");
		}
	}
}

void grd_print_stack_trace(GrdStackTrace st) {
	char buf[4 * 1024];
	buf[0] = '\0';
	grd_print_stack_trace(st, buf, sizeof(buf));
	printf("%s\n", buf);
}
