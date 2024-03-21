#pragma once

#include "../../base.h"

#include <Windows.h>

using OsMutex = CRITICAL_SECTION;

void os_mutex_create(OsMutex* mutex) {
	*mutex = {};
	InitializeCriticalSection(mutex);
}

void os_mutex_lock(OsMutex* mutex) {
	EnterCriticalSection(mutex);
}

void os_mutex_unlock(OsMutex* mutex) {
	LeaveCriticalSection(mutex);
}

void os_mutex_destroy(OsMutex* mutex) {
	DeleteCriticalSection(mutex);
}


using OsSemaphore = HANDLE;

void os_semaphore_create(OsSemaphore* sem, u32 initial_value) {
	*sem = CreateSemaphoreA(NULL, initial_value, LONG_MAX, NULL);
}

void os_semaphore_wait_and_decrement(OsSemaphore* sem) {
	WaitForSingleObject(*sem, INFINITE);
}

void os_semaphore_increment(OsSemaphore* sem) {
	long previous_count;
	ReleaseSemaphore(*sem, 1, &previous_count);
}

void os_semaphore_destroy(OsSemaphore* sem) {
	CloseHandle(*sem);
}
