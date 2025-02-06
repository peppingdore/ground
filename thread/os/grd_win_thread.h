#pragma once

#include "../../grd_base.h"
#include "grd_win32_api.h"

using GrdThreadId = DWORD;

GRD_DEDUP void grd_os_sleep(u32 ms) {
	Sleep(ms);
}

GRD_DEDUP GrdThreadId grd_current_thread_id() {
	return (GrdThreadId) GetCurrentThreadId();
}

using GrdOsThread = HANDLE;
using GrdOsThreadReturnType = DWORD;

GRD_DEDUP bool grd_os_thread_start(GrdOsThread* thread, GrdOsThreadReturnType (WINAPI *proc)(void*), void* data) {
	DWORD thread_id;
	HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) proc, data, 0, &thread_id);
	if (!handle) {
		return false;
	}
	*thread = handle;
	return true;
}

GRD_DEDUP void grd_os_thread_join(GrdOsThread* thread) {
	WaitForSingleObject(*thread, INFINITE);
}

GRD_DEDUP GrdThreadId grd_os_thread_get_id(GrdOsThread* thread) {
	return GetThreadId(*thread);
}
