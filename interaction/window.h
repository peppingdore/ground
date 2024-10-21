#pragma once

#include "../grd_base.h"

#if GRD_OS_WINDOWS
	#include "grd_win_window.h"
#endif

GrdTuple<GrdWindow*, GrdError*> create_window(GrdWindowParams params) {
	auto [window, e] = grd_os_create_window(params);
	return { window, e };
}

GrdArray<GrdEvent*> read_window_events(GrdWindow* window) {
	return grd_os_read_window_events((GrdWindowsWindow*) window);
}