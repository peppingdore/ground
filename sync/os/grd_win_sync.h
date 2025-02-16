#pragma once

#include "../../grd_base.h"

#include "grd_win32_api.h"
// #include <limits>

using GrdOsMutex = GRD_WIN32_CRITICAL_SECTION;

GRD_DEDUP void grd_os_mutex_create(GrdOsMutex* mutex) {
	*mutex = {};
	InitializeCriticalSection(mutex);
}

GRD_DEDUP void grd_os_mutex_lock(GrdOsMutex* mutex) {
	EnterCriticalSection(mutex);
}

GRD_DEDUP void grd_os_mutex_unlock(GrdOsMutex* mutex) {
	LeaveCriticalSection(mutex);
}

GRD_DEDUP void grd_os_mutex_destroy(GrdOsMutex* mutex) {
	DeleteCriticalSection(mutex);
}


using GrdOsSemaphore = GRD_WIN_HANDLE;

GRD_DEDUP void grd_os_semaphore_create(GrdOsSemaphore* sem, u32 initial_value) {
	*sem = CreateSemaphoreA(NULL, initial_value, 2147483647L, NULL);
}

GRD_DEDUP void grd_os_semaphore_wait_and_decrement(GrdOsSemaphore* sem) {
	WaitForSingleObject(*sem, GRD_WIN_INFINITE);
}

GRD_DEDUP void grd_os_semaphore_increment(GrdOsSemaphore* sem) {
	long previous_count;
	ReleaseSemaphore(*sem, 1, &previous_count);
}

GRD_DEDUP void grd_os_semaphore_destroy(GrdOsSemaphore* sem) {
	CloseHandle(*sem);
}
