#include "../grd_window.h"
#include "../../grd_testing.h"
#include "../../grd_tracker_allocator.h"

GRD_TEST_CASE(window_open_close) {
	auto [window, e] = grd_create_window({ .title = U"Window"_b });
	GRD_EXPECT(!e, e);
	GRD_EXPECT(window->type);
	
	grd_close_window(window);
}
