#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
#pragma once

#include "../grd_stack_trace.h"
#include "../grd_log.h"
#include "../grd_testing.h"

GRD_TEST_CASE(stack_trace) {
	auto cs = grd_get_stack_trace();
	grd_print_stack_trace(cs);
	GRD_EXPECT(false);
}
