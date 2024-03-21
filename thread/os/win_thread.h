#pragma once

#include "../../base.h"
#include <Windows.h>

using ThreadId = DWORD;

void os_sleep(u32 ms) {
	Sleep(ms);
}

ThreadId current_thread_id() {
	return (ThreadId) GetCurrentThreadId();
}

using OsThread = HANDLE;
using OsThreadReturnType = DWORD;

bool os_thread_start(OsThread* thread, OsThreadReturnType (WINAPI *proc)(void*), void* data) {
	DWORD thread_id;
	HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) proc, data, 0, &thread_id);
	if (!handle) {
		return false;
	}
	*thread = handle;
	return true;
}

void os_thread_join(OsThread* thread) {
	WaitForSingleObject(*thread, INFINITE);
}

ThreadId os_thread_get_id(OsThread* thread) {
	return GetThreadId(*thread);
}
