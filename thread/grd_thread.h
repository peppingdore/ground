#pragma once

#include "../sync/grd_atomics.h"
#include "../grd_panic.h"
#include "os/grd_os_thread.h"

struct GrdThread {
	void*       data;
	GrdOsThread os_thread;

	// @TODO: demethodize.
	void join() {
		grd_os_thread_join(&os_thread);
	}

	GrdThreadId get_id() {
		return grd_os_thread_get_id(&os_thread);
	}
};

template <typename... Args>
GRD_DEDUP GrdThread grd_start_thread(auto* proc, Args ...args) {
	auto args_tuple = grd_make_tuple(args...);

    GrdThread thread;
	struct ThreadData {
		decltype(proc)       proc;
		decltype(args_tuple) args;
	};

	auto data = grd_make<ThreadData>(c_allocator);
	data->proc = proc;
	data->args = args_tuple;

	auto thread_main = [](void* data_ptr) -> GrdOsThreadReturnType {
		auto* data = (ThreadData*) data_ptr;
		grd_call_with_tuple(*data->proc, data->args);
		GrdFree(c_allocator, data);
		return {};
	};
	thread.data = data;

	bool ok = grd_os_thread_start(&thread.os_thread, thread_main, thread.data);
	if (!ok) {
		grd_panic("failed to start a thread");
	}
	return thread;
}
