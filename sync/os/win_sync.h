#pragma once

#include "../../base.h"

#include <Windows.h>

using Thread_Id = DWORD;

void os_sleep(u32 ms) {
	Sleep(ms);
}

Thread_Id current_thread_id() {
	return (Thread_Id) GetCurrentThreadId();
}


using Os_Mutex = CRITICAL_SECTION;

void os_mutex_create(Os_Mutex* mutex) {
	*mutex = {};
	InitializeCriticalSection(mutex);
}

void os_mutex_lock(Os_Mutex* mutex) {
	EnterCriticalSection(mutex);
}

void os_mutex_unlock(Os_Mutex* mutex) {
	LeaveCriticalSection(mutex);
}

void os_mutex_destroy(Os_Mutex* mutex) {
	DeleteCriticalSection(mutex);
}


using Os_Semaphore = HANDLE;

void os_semaphore_create(Os_Semaphore* sem, u32 initial_value) {
	*sem = CreateSemaphoreA(NULL, initial_value, LONG_MAX, NULL);
}

void os_semaphore_wait_and_decrement(Os_Semaphore* sem) {
	WaitForSingleObject(*sem, INFINITE);
}

void os_semaphore_increment(Os_Semaphore* sem) {
	long previous_count;
	ReleaseSemaphore(*sem, 1, &previous_count);
}

void os_semaphore_destroy(Os_Semaphore* sem) {
	CloseHandle(*sem);
}


using Os_Thread = HANDLE;
using Os_Thread_Return_Type = DWORD;

bool os_thread_start(Os_Thread* thread, Os_Thread_Return_Type (WINAPI *proc)(void*), void* data) {
	DWORD thread_id;
	HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) proc, data, 0, &thread_id);
	if (!handle) {
		return false;
	}
	*thread = handle;
	return true;
}

void os_thread_join(Os_Thread* thread) {
	WaitForSingleObject(*thread, INFINITE);
}

Thread_Id os_thread_get_id(Os_Thread* thread) {
	return GetThreadId(*thread);
}
