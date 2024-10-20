#pragma once

#include "../grd_base.h"

#if GRD_OS_WINDOWS
	#include "win_window.h"
#endif

Tuple<Window*, Error*> create_window(WindowParams params) {
	auto [window, e] = os_create_window(params);
	return { window, e };
}

GrdArray<Event*> read_window_events(Window* window) {
	return os_read_window_events((WindowsWindow*) window);
}