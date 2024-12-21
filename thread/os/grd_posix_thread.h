#pragma once

#include <pthread.h>
#include <unistd.h>

using GrdThreadId = pthread_t;

GRD_DEDUP void grd_os_sleep(u32 ms) {
	usleep(ms * 1000);
}

GRD_DEDUP GrdThreadId grd_current_thread_id() {
	return pthread_self();
}

using GrdOsThread = pthread_t;
using GrdOsThreadReturnType = void*;

GRD_DEDUP bool grd_os_thread_start(GrdOsThread* thread, GrdOsThreadReturnType (*proc)(void*), void* data) {
	pthread_t handle;
	int result = pthread_create(&handle, NULL, proc, data);
	if (result != 0) {
		return false;
	}
	*thread = handle;
	return true;
}

GRD_DEDUP void grd_os_thread_join(GrdOsThread* thread) {
	pthread_join(*thread, NULL);
}

GRD_DEDUP GrdThreadId grd_os_thread_get_id(GrdOsThread* thread) {
	return *thread;
}
