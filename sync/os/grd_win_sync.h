#pragma once

#include "../../grd_base.h"

#include <Windows.h>

using GrdOsMutex = CRITICAL_SECTION;

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


using GrdOsSemaphore = HANDLE;

GRD_DEDUP void grd_os_semaphore_create(GrdOsSemaphore* sem, u32 initial_value) {
	*sem = CreateSemaphoreA(NULL, initial_value, LONG_MAX, NULL);
}

GRD_DEDUP void grd_os_semaphore_wait_and_decrement(GrdOsSemaphore* sem) {
	WaitForSingleObject(*sem, INFINITE);
}

GRD_DEDUP void grd_os_semaphore_increment(GrdOsSemaphore* sem) {
	long previous_count;
	ReleaseSemaphore(*sem, 1, &previous_count);
}

GRD_DEDUP void grd_os_semaphore_destroy(GrdOsSemaphore* sem) {
	CloseHandle(*sem);
}
