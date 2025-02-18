#include "../grd_window.h"
#include "../../grd_testing.h"
#include "../../grd_tracker_allocator.h"

GRD_TEST_CASE(window_test) {
	grd_get_logger();
	c_allocator = grd_make_tracker_allocator(c_allocator);

	auto [window, e] = grd_create_window({ .title = U"Window"_b });
	GRD_EXPECT(!e, e);
	GRD_EXPECT(window->type);
	
	grd_close_window(window);

	GRD_EXPECT(grd_tracker_allocator_is_empty(c_allocator));
	if (!grd_tracker_allocator_is_empty(c_allocator)) {
		grd_tracker_allocator_print_usage(c_allocator);
	}
}
