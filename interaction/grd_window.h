#pragma once

#include "../grd_base.h"

#if GRD_OS_WINDOWS
	#include "grd_win_window.h"
#endif

GRD_DEDUP GrdTuple<GrdWindow*, GrdError*> grd_create_window(GrdWindowParams params) {
	auto [window, e] = grd_os_create_window(params);
	return { window, e };
}

GRD_DEDUP GrdArray<GrdEvent*> read_window_events(GrdWindow* window) {
	return grd_os_read_window_events((GrdWindowsWindow*) window);
}

GRD_DEF grd_close_window(GrdWindow* window) {
	grd_os_close_window(window);
	GrdFree(window);
}
