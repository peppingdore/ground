#pragma once

#include <pthread.h>
#include <unistd.h>

using GrdThreadId = pthread_t;

void grd_os_sleep(u32 ms) {
	usleep(ms * 1000);
}

GrdThreadId grd_current_thread_id() {
	return pthread_self();
}

using GrdOsThread = pthread_t;
using GrdOsThreadReturnType = void*;

bool grd_os_thread_start(GrdOsThread* thread, GrdOsThreadReturnType (*proc)(void*), void* data) {
	pthread_t handle;
	int result = pthread_create(&handle, NULL, proc, data);
	if (result != 0) {
		return false;
	}
	*thread = handle;
	return true;
}

void grd_os_thread_join(GrdOsThread* thread) {
	pthread_join(*thread, NULL);
}

GrdThreadId grd_os_thread_get_id(GrdOsThread* thread) {
	return *thread;
}
