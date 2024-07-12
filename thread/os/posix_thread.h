#pragma once

#include <pthread.h>
#include <unistd.h>

using ThreadId = pthread_t;

void os_sleep(u32 ms) {
	usleep(ms * 1000);
}

ThreadId current_thread_id() {
	return pthread_self();
}

using OsThread = pthread_t;
using OsThreadReturnType = void*;

bool os_thread_start(OsThread* thread, OsThreadReturnType (*proc)(void*), void* data) {
	pthread_t handle;
	int result = pthread_create(&handle, NULL, proc, data);
	if (result != 0) {
		return false;
	}
	*thread = handle;
	return true;
}

void os_thread_join(OsThread* thread) {
	pthread_join(*thread, NULL);
}

ThreadId os_thread_get_id(OsThread* thread) {
	return *thread;
}
