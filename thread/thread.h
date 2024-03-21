#pragma once

#include "../sync/atomics.h"
#include "../panic.h"
#include "os/os_thread.h"

struct Thread {
	void*    data;
	OsThread os_thread;

	void join() {
		os_thread_join(&os_thread);
	}

	ThreadId get_id() {
		return os_thread_get_id(&os_thread);
	}
};

template <typename... Args>
Thread start_thread(auto* proc, Args ...args) {
	auto args_tuple = make_tuple(args...);

    Thread thread;
	struct ThreadData {
		decltype(proc)       proc;
		decltype(args_tuple) args;
	};

	auto data = make<ThreadData>(c_allocator);
	data->proc = proc;
	data->args = args_tuple;

	auto thread_main = [](void* data_ptr) -> OsThreadReturnType {
		ZoneScopedN("thread_main");
		auto* data = (ThreadData*) data_ptr;
		call_with_tuple(*data->proc, data->args);
		free(c_allocator, data);
		return {};
	};
	thread.data = data;

	bool ok = os_thread_start(&thread.os_thread, thread_main, thread.data);
	if (!ok) {
		panic("failed to start a thread");
	}
	return thread;
}
